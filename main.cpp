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

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "cue.h"
#include "discogs.h"
#include "filename_utility.h"
#include "json.hpp"
#include "naming.h"
#include "string_utility.h"
#include "support_types.h"

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

std::string concatenate_artists(const nlohmann::json& artists) {
  std::string rv;
  for (auto&& artist_info : artists) {
    auto name = artist_info.value("anv", std::string());
    if (name.empty()) {
      name = artist_info.value("name", std::string());
    }
    NamingFacets::artist_facets(name);
    rv += name;
    if (&artist_info != &artists.back()) {
      auto join = artist_info.value("join", std::string());
      if (join == ",") {
        rv += join;
      } else {
        rv += " ";
      }
    }
  }
  return rv;
}

bool fetch(const std::string& id, nlohmann::json& data,
           const bool is_master = false) {
  DiscogsReleaseRequest req;
  return req.send(id, data, is_master);
}

bool get_disc_number(const std::string& position, unsigned& discno) {
  auto dotpos = position.find_first_of(".-");
  if (dotpos == std::string::npos || (dotpos + 1) == position.length()) {
    return false;
  }
  discno = string_to_numeric<unsigned>(position.substr(0, dotpos));
  try {
    // string_to_numeric has no facility to notify of failure, but we need to be
    // sure if this is actually a "2.3"-style track position to return true.
    std::stoul(position.substr(dotpos + 1));
    return true;
  } catch (...) {
  }
  return false;
}

void generate(const nlohmann::json& toplevel, const std::string& filename) {
  Album a;
  a.title = toplevel.value("title", std::string());
  auto year = toplevel.value("year", -1);
  if (year != -1) {
    a.year = std::to_string(year);
  }
  // style maps to genre better than genre does, in general
  if (toplevel.find("styles") != toplevel.end()) {
    a.genre = toplevel["styles"][0].get<std::string>();
  }
  if (toplevel.find("artists") != toplevel.end()) {
    a.album_artist = concatenate_artists(toplevel["artists"]);
  }

  Disc d;
  a.discs.push_back(d);
  unsigned disc = 0;
  unsigned track_num = 1;
  for (auto&& track_info : toplevel.at("tracklist")) {
    auto position = track_info.value("position", std::string());
    if (position.empty()) {
      continue;
    }
    unsigned this_disc;
    if (get_disc_number(position, this_disc) && this_disc > disc) {
      ++disc;
      Disc nd;
      a.discs.push_back(nd);
      track_num = 1;
    }
    Track t;
    t.position = track_num;
    if (track_info.find("artists") != track_info.end()) {
      t.artist = concatenate_artists(track_info["artists"]);
    } else {
      t.artist = a.album_artist;
    }
    t.title = track_info.value("title", std::string());
    auto duration = track_info.value("duration", std::string());
    if (duration.empty()) {
      std::cerr << "Track " << track_num << ", disc " << disc
                << " has no duration, quitting\n";
      ::exit(1);
    }
    std::vector<std::string> time_components;
    explode(duration, ":", time_components);
    if (time_components.size() == 2) {
      trim(time_components[0]);
      trim(time_components[1]);
      t.length.min = string_to_numeric<unsigned>(time_components[0]);
      t.length.sec = string_to_numeric<unsigned>(time_components[1]);
    } else {
      std::cerr << "Unrecognised duration " << duration << ", quitting\n";
      ::exit(1);
    }
    ++track_num;
    a.discs[disc].tracks.push_back(t);
  }

  // if multidisc album, we have to remove the useless disc we created in the
  // loop
  if (a.discs.size() > 1) {
    a.discs.erase(a.discs.begin());
  }

  try {
    Cue_build(a, filename);
  } catch (std::runtime_error& e) {
    std::cerr << e.what() << '\n';
    exit(1);
  }
}

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
}

int main(int argc, char* argv[]) {
  const auto argv_end = (argv + argc);
  auto need_help = std::find_if(argv, argv_end, [](char* str) {
    return ::strcmp(str, "--help") == 0 || ::strcmp(str, "-h") == 0 ||
           ::strcmp(str, "-H") == 0;
  });
  if (argc < 3 || (need_help != argv_end)) {
    std::cerr << help << '\n';
    return 0;
  }

  bool do_cover = false;
  std::string cover_fname = "cover.jpg";
#ifdef DCUE_OFFICIAL_BUILD
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

  generate(discogs_data, argv[2]);

  if (do_cover) {
    get_cover(discogs_data, cover_fname);
  }
  return 0;
}
