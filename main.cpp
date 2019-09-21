// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2019 Daniel Kamil Kozar
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
const char help[] =
    "********" COMMENT "********"
    "\n"
    "DCue is a cue sheet generator which uses Discogs.com to find track "
    "titles, lengths and other information."
    "\n"
    "\n"
    "SYNTAX:"
    "\n"
    "dcue [(r)elease=|(m)aster=]<id> <audio filename>"
    "\n"
    "\n"
    "FIRST ARGUMENT: a Discogs release or master release ID. Specify "
    "\"release=<id>\" or \"r=<id>\" or just \"<id>\" for a regular release and "
    "\"master=<id>\" or \"m=<id>\" for a master."
    "\n"
    "SECOND ARGUMENT: filename with optional absolute path of the AUDIO FILE "
    "you want to make a cue for. The cue file will be created alongside it. "
    "\"?\" characters will be replaced by the disc number."
    "\n"
    "\n"
    "EXAMPLES:"
    "\n"
    "dcue master=218406 \"Clubland X-Treme Hardcore-Disc?.wav\""
    "\n"
    "dcue r=1 \"/path/to/the punisher - stockholm.mp3\""
    "\n"
    "dcue 1432 \"Release filename.flac\""
    "\n"
    "\n"
    "OPTIONS:"
    "\n"
    "--help (-h) - this command list"
    "\n";

const char error[] = "Invalid syntax, use --help for help";

template <typename T>
T get_if_exists(const nlohmann::json& json, const char* key, T defval = T()) {
  return json.find(key) != json.end() ? json[key].get<T>() : defval;
}

std::string concatenate_artists(const nlohmann::json& artists) {
  std::string artist;
  for (auto&& artist_info : artists) {
    auto anv = get_if_exists<std::string>(artist_info, "anv");
    if (!anv.empty()) {
      artist += anv;
    } else {
      artist += artist_info["name"];
    }
    NamingFacets::artist_facets(artist);
    auto join = get_if_exists<std::string>(artist_info, "join");
    if (!join.empty()) {
      if (join != ",") {
        artist += " ";
      }
      artist += join;
    }
    artist += " ";
  }
  artist.erase(artist.length() - 1, 1);
  return artist;
}

void generate(const std::string& id, const std::string& filename,
              const bool is_master = false) {
  std::string json;
  DiscogsReleaseRequest req;
  if (!req.send(id, json, is_master)) {
    std::cerr
        << "Failed to get valid release info from Discogs (are you "
           "connected to the internet? are you sure the ID is correct?)\n";
    std::exit(1);
  }
  nlohmann::json toplevel = nlohmann::json::parse(json);
  Album a;
  a.title = get_if_exists<std::string>(toplevel, "title");
  if (toplevel.find("year") != toplevel.end()) {
    a.year = std::to_string(toplevel["year"].get<int>());
  }
  // style maps to genre better than genre does, in general
  if (toplevel.find("styles") != toplevel.end()) {
    a.genre = toplevel["styles"][0];
  }
  if (toplevel.find("artists") != toplevel.end()) {
    a.album_artist = concatenate_artists(toplevel["artists"]);
  }

  Disc d;
  a.discs.push_back(d);
  unsigned disc = 0;
  unsigned track_num = 1;
  for (auto&& track_info : toplevel["tracklist"]) {
    auto position = track_info["position"].get<std::string>();
    if (position.empty()) {
      // ++disc;
      // Disc nd;
      // a.discs.push_back(nd);
      // track_num = 1;
      continue;
    }
    if (position.find_first_of(".-") != std::string::npos &&
        string_to_numeric<unsigned>(
            position.substr(0, position.find_first_of(".-"))) > disc) {
      ++disc;
      Disc nd;
      a.discs.push_back(nd);
      track_num = 1;
    }
    Track t;
    t.position = track_num;
    ++track_num;
    if (track_info.find("artists") != track_info.end()) {
      t.artist = concatenate_artists(track_info["artists"]);
    } else {
      t.artist = a.album_artist;
    }
    t.title = get_if_exists<std::string>(track_info, "title");
    if (track_info.find("duration") != track_info.end()) {
      std::vector<std::string> time_components;
      explode(track_info["duration"], ":", time_components);
      if (time_components.size() == 2) {
        trim(time_components[0]);
        trim(time_components[1]);
        t.length.min = string_to_numeric<unsigned>(time_components[0]);
        t.length.sec = string_to_numeric<unsigned>(time_components[1]);
      }
    }
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
}

int main(int argc, char* argv[]) {
  std::string first;
  if (argc < 2) {
    std::cerr << help << '\n';
    return 1;
  } else {
    first = argv[1];
  }

  if (first == "--help" || first == "-h" || first == "-H") {
    std::cerr << help << '\n';
    return 0;
  } else {
    if (argc != 3) {
      std::cerr << error << '\n';
      return 1;
    } else {
      std::string rel = first;
      std::transform(rel.begin(), rel.end(), rel.begin(), ::tolower);
      std::string fn(argv[2]);
      std::string single = rel.substr(0, 2);
      std::string full = rel.substr(0, 8);
      std::string mfull = rel.substr(0, 7);
      if (rel.find("=") == std::string::npos) {
        generate(rel, fn);
      } else if (single == "r=" || full == "release=") {
        generate(rel.substr(rel.find("=") + 1), fn);
      } else if (single == "m=" || mfull == "master=") {
        generate(rel.substr(rel.find("=") + 1), fn, true);
      } else {
        std::cerr << error << '\n';
        return 1;
      }
    }
  }
  return 0;
}
