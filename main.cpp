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

#include <fstream>
#include <string>
#include <string_view>

#include "album.h"
#include "cue.h"
#include "discogs.h"
#include "http.h"
#include "multitrack_strategy.h"

#include <nlohmann/json.hpp>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

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
--medley (single|merge|separate) - set medley track handling
--index (single|merge|separate) - set index track handling
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
void get_cover(const nlohmann::json& toplevel, std::string_view fname) {
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

  std::ofstream outfile(fname.data(), std::ios::out | std::ios::binary);
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

std::unique_ptr<multitrack_strategy>
get_strategy(const std::vector<std::string_view>& args,
             std::string_view option) {
  auto found_arg_it = std::find(std::cbegin(args), std::cend(args), option);
  if (found_arg_it == std::cend(args)) {
    return multitrack_strategy::single();
  }
  auto value_it = found_arg_it + 1;
  if (value_it == std::cend(args)) {
    return multitrack_strategy::single();
  }

  using namespace std::string_view_literals;
  if (*value_it == "single"sv) {
    return multitrack_strategy::single();
  } else if (*value_it == "merge"sv) {
    return multitrack_strategy::merge();
  } else if (*value_it == "separate"sv) {
    return multitrack_strategy::separate();
  } else {
    SPDLOG_WARN("Unrecognised value {} for option {} : assuming 'single'",
                *value_it, option);
    return multitrack_strategy::single();
  }
}

int real_main(const std::vector<std::string_view>& args) {
  using namespace std::string_view_literals;
  auto need_help =
      std::find_if(std::cbegin(args), std::cend(args), [](auto sv) {
        return sv == "--help"sv || sv == "-h"sv || sv == "-H"sv;
      });
  if (args.size() < 3 || (need_help != std::cend(args))) {
    SPDLOG_INFO("{}", help);
    return 0;
  }

#ifdef DCUE_OFFICIAL_BUILD
  bool do_cover = false;
  std::string_view cover_fname = "cover.jpg"sv;
  {
    const auto cover_arg =
        std::find(std::cbegin(args), std::cend(args), "--cover"sv);
    do_cover = (cover_arg != std::cend(args));
    const auto cover_fname_arg =
        std::find(std::cbegin(args), std::cend(args), "--cover-file"sv);
    if (cover_fname_arg != std::cend(args)) {
      const auto actual_cover_fname_arg = cover_fname_arg + 1;
      if (actual_cover_fname_arg != std::cend(args)) {
        cover_fname = *actual_cover_fname_arg;
      }
    }
  }
#endif

  auto index_strategy = get_strategy(args, "--index"sv);
  auto medley_strategy = get_strategy(args, "--medley"sv);

  auto rel = std::string{args[1]};
  std::transform(rel.begin(), rel.end(), rel.begin(), ::tolower);
  try {
    auto req = DiscogsRequestFactory::create(rel);
    auto resp = req.send();
    if (!resp || resp->status != HttpStatus::OK) {
      SPDLOG_ERROR(
          "Failed to get valid release info from Discogs (are you "
          "connected to the internet? are you sure the ID is correct?)");
      return 1;
    }
    const auto discogs_data = nlohmann::json::parse(resp->body);
    const auto album =
        Album::from_json(discogs_data, *index_strategy, *medley_strategy);
    cue::generate(album, args[2], [](auto& path) {
      return std::make_shared<std::ofstream>(path,
                                             std::ios::binary | std::ios::out);
    });
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
}

int main(int argc, char** argv) {
  spdlog::cfg::load_env_levels();
#ifdef DCUE_OFFICIAL_BUILD
  spdlog::set_pattern("%v");
#endif
  ::HttpInit i__;
  std::vector<std::string_view> args;
  args.reserve(argc);
  std::for_each(argv, argv + argc, [&](char* arg) { args.emplace_back(arg); });
  return real_main(args);
}
