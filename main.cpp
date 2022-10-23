// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2019-2022 Daniel Kamil Kozar
// Original version by :
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "defs.h"

#include <cstring>
#include <iostream>
#include <string>

#include "cue.h"
#include "discogs.h"
#include "http.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>

namespace {
const char help[] = "********" COMMENT "********\n"
                    R"helpstr(
DCue is a cue sheet generator which uses Discogs.com to find track titles,
lengths and other information.

SYNTAX:
dcue [(r)elease=|(m)aster=]<id> <audio filename> [options]

FIRST ARGUMENT: a Discogs release or master release ID. Specify "release=<id>"
or "r=<id>" or just "<id>" for a regular release and "master=<id>" or "m=<id>"
for a master.

SECOND ARGUMENT: filename with optional absolute path of the AUDIO FILE you want
to make a cue for. The cue file will be created alongside it.
"?" characters will be replaced by the disc number.

EXAMPLES:
dcue master=218406 "Clubland X-Treme Hardcore-Disc?.wav"
dcue r=1 "/path/to/the punisher - stockholm.mp3"
dcue 1432 "Release filename.flac"

MEDLEY AND INDEX TRACK HANDLING:
Medley tracks (also called hidden tracks) are tracks which appear in the track
listing provided by Discogs with no duration and a track number like "3.1",
"A3.a" or "A3b". They are a part of the track that the first number/token in
their position refer to.

Index tracks are essentially the same thing as far as the end-user is concerned
but a bit more formalised as they appear as separate entities in Discogs' JSON
output and its release pages.

How medley and index tracks are represented in the CUE output can be customised
by passing one of those strings to the --medley or --index options :
 * single : Use the index track's or the referenced track's (in case of medley
            tracks) artist/title information for the whole index/medley track.
 * merge : Merge artist/title information from all index/medley tracks into
           artist/title fields in the CUE.
 * separate : Output all index/medley tracks as separate CUE tracks. This is
              only possible if they all have assigned durations and will cause
              an error otherwise.

"single" is used for both if unspecified.

OPTIONS:
--medley=(single|merge|separate) - set medley track handling
--index=(single|merge|separate) - set index track handling
--help / -h - this command list
)helpstr"

#ifdef DCUE_OFFICIAL_BUILD
                    R"helpstr(
--cover - fetch the cover for this release
  (the "primary" cover is preferred, otherwise the first one available is
  fetched)

--cover-file filename.jpg - name of the file to save the cover to
  (default is cover.jpg)
)helpstr"
#endif
    ;

#ifdef DCUE_OFFICIAL_BUILD
#include "appkey.h"
void get_cover(const nlohmann::json& toplevel, const std::string& fname) {
  auto it = toplevel.find("images");
  if (it == toplevel.end()) {
    SPDLOG_INFO("Unfortunately, this release has no associated cover images");
    return;
  }
  auto&& images = *it;
  // look for a "primary" image first...
  auto imageIt =
      std::find_if(images.begin(), images.end(), [](const nlohmann::json& img) {
        return img.value("type", "") == "primary" &&
               (!img.value("uri", "").empty());
      });
  if (imageIt == images.end()) {
    // settle for any image that has an URI we can hopefully get.
    imageIt = std::find_if(images.begin(), images.end(),
                           [](const nlohmann::json& img) {
                             return !img.value("uri", "").empty();
                           });
  }
  if (imageIt == images.end()) {
    SPDLOG_INFO("No suitable images found");
    return;
  }

  std::ofstream outfile(fname, std::ios::out | std::ios::binary);
  if (!outfile.is_open()) {
    SPDLOG_WARN("Failed to create/open file {}", fname);
    return;
  }

  HttpGet req;
  const auto uri = imageIt->at("uri").get<std::string>();
  req.set_resource(uri);
  req.add_header(discogs_key::getHeader());
  if (const auto rsp = req.send()) {
    outfile.write(reinterpret_cast<const char*>(rsp->body.data()),
                  rsp->body.size());
  } else {
    SPDLOG_WARN("Image request failed");
  }
}
#endif

struct HttpInit {
  HttpInit() {
    HttpGet::global_init();
  }
  ~HttpInit() {
    HttpGet::global_deinit();
  }
  HttpInit(const HttpInit&) = delete;
  HttpInit& operator=(const HttpInit&) = delete;
};
}

int main(int argc, char* argv[]) {
  spdlog::cfg::load_env_levels();
  ::HttpInit i__;
  const auto argv_end = (argv + argc);
  auto need_help = std::find_if(argv, argv_end, [](char* str) {
    return ::strcmp(str, "--help") == 0 || ::strcmp(str, "-h") == 0 ||
           ::strcmp(str, "-H") == 0;
  });
  if (argc < 3 || (need_help != argv_end)) {
    SPDLOG_INFO("{}", help);
    return 0;
  }

#ifdef DCUE_OFFICIAL_BUILD
  bool do_cover = false;
  std::string cover_fname = "cover.jpg";
  {
    const auto cover_arg = std::find_if(argv, argv_end, [](char* str) {
      return ::strcmp(str, "--cover") == 0;
    });
    do_cover = (cover_arg != argv_end);
    const auto cover_fname_arg = std::find_if(argv, argv_end, [](char* str) {
      return ::strcmp(str, "--cover-file") == 0;
    });
    if (cover_fname_arg != argv_end) {
      const auto actual_cover_fname_arg = cover_fname_arg + 1;
      if (actual_cover_fname_arg != argv_end) {
        cover_fname = *actual_cover_fname_arg;
      }
    }
  }
#endif

  std::string rel = argv[1];
  std::transform(rel.begin(), rel.end(), rel.begin(), ::tolower);
  try {
    auto req = DiscogsRequestFactory::create(rel);
    auto resp = req.send();
    if (!resp || resp->status != HttpStatus::OK) {
      SPDLOG_ERROR("Failed to get valid release info from Discogs (are you "
             "connected to the internet? are you sure the ID is correct?)");
      return 1;
    }
    const auto discogs_data = nlohmann::json::parse(resp->body);
    generate(discogs_data, argv[2]);
#ifdef DCUE_OFFICIAL_BUILD
    if (do_cover) {
      get_cover(discogs_data, cover_fname);
    }
#endif
    return 0;
  } catch (const std::exception& e) {
    SPDLOG_ERROR("{}", e.what());
  }
  return 1;
}
