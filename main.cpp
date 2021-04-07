// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2019-2021 Daniel Kamil Kozar
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
#include <fstream>
#include <iostream>
#include <string>

#include "cue.h"
#include "discogs.h"
#include "json.hpp"

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

OPTIONS:
--help (-h) - this command list
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

const char error[] = "Invalid syntax, use --help for help";

bool fetch(const std::string& id, nlohmann::json& data,
           const bool is_master = false) {
  DiscogsReleaseRequest req;
  return req.send(id, data, is_master);
}

#ifdef DCUE_OFFICIAL_BUILD
#include "appkey.h"
void get_cover(const nlohmann::json& toplevel, const std::string& fname) {
  auto it = toplevel.find("images");
  if (it == toplevel.end()) {
    std::cerr << "Unfortunately, this release has no associated cover images\n";
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
    std::cerr << "No suitable images found\n";
    return;
  }

  HttpGet g;
  g.set_resource((*imageIt).at("uri"));
  g.add_header(discogs_key::getHeader());
  HttpResponse out;
  if (!g.send("", out)) {
    std::cerr << "Image request failed\n";
    return;
  }

  std::ofstream outfile(fname, std::ios::out | std::ios::binary);
  if (!outfile.is_open()) {
    std::cerr << "Failed to create/open file " << fname << '\n';
    return;
  }
  outfile.write(reinterpret_cast<const char*>(out.body.data()),
                out.body.size());
}
#endif
}

int main(int argc, char* argv[]) {
  HttpGet::global_init();
  const auto argv_end = (argv + argc);
  auto need_help = std::find_if(argv, argv_end, [](char* str) {
    return ::strcmp(str, "--help") == 0 || ::strcmp(str, "-h") == 0 ||
           ::strcmp(str, "-H") == 0;
  });
  if (argc < 3 || (need_help != argv_end)) {
    std::cerr << help << '\n';
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
  std::string single = rel.substr(0, 2);
  std::string full = rel.substr(0, 8);
  std::string mfull = rel.substr(0, 7);
  nlohmann::json discogs_data;
  bool fetch_ok = false;
  if (rel.find("=") == std::string::npos) {
    fetch_ok = fetch(rel, discogs_data);
  } else if (single == "r=" || full == "release=") {
    fetch_ok = fetch(rel.substr(rel.find("=") + 1), discogs_data);
  } else if (single == "m=" || mfull == "master=") {
    fetch_ok = fetch(rel.substr(rel.find("=") + 1), discogs_data, true);
  } else {
    std::cerr << error << '\n';
    return 1;
  }

  if (!fetch_ok) {
    std::cerr
        << "Failed to get valid release info from Discogs (are you "
           "connected to the internet? are you sure the ID is correct?)\n";
    return 1;
  }

  try {
    generate(discogs_data, argv[2]);
#ifdef DCUE_OFFICIAL_BUILD
    if (do_cover) {
      get_cover(discogs_data, cover_fname);
    }
#endif
    return 0;
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
  }
  HttpGet::global_deinit();
  return 1;
}
