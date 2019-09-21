// *******************************************************************
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
#include "json_parser.h"
#include "naming.h"
#include "string_utility.h"
#include "support_types.h"

using std::cerr;
using std::cout;
using std::endl;
using std::strcmp;
using std::string;
using std::transform;

namespace {
const string help =
    "********" COMMENT "********" LINE_END
    "DCue is a cue sheet generator which uses Discogs.com to find track "
    "titles, lengths and other information." LINE_END LINE_END
    "SYNTAX:" LINE_END
    "dcue [(r)elease=|(m)aster=]<id> <audio filename>" LINE_END LINE_END
    "FIRST ARGUMENT: a Discogs release or master release ID. Specify "
    "\"release=<id>\" or \"r=<id>\" or just \"<id>\" for a regular release and "
    "\"master=<id>\" or \"m=<id>\" for a master." LINE_END
    "SECOND ARGUMENT: filename with optional absolute path of the AUDIO FILE "
    "you want to make a cue for. The cue file will be created alongside it. "
    "\"?\" characters will be replaced by the disc number." LINE_END LINE_END
    "EXAMPLES:" LINE_END
    "dcue master=218406 \"Clubland X-Treme Hardcore-Disc?.wav\"" LINE_END
    "dcue r=1 \"/path/to/the punisher - stockholm.mp3\"" LINE_END
    "dcue 1432 \"Release filename.flac\"" LINE_END LINE_END "OPTIONS:" LINE_END
    "--help (-h) - this command list" LINE_END;

const string error = "Invalid syntax, use --help for help";

void generate(const std::string& id, const std::string& filename,
              const bool is_master = false) {
  std::string json;
  DiscogsReleaseRequest req;
  if (!req.send(id, json, is_master)) {
    std::cerr << "Failed to get valid release info from Discogs (are you "
                 "connected to the internet? are you sure the ID is correct?)"
              << std::endl;
    std::exit(1);
  }
  string_map fields;
  JsonParser::parse_object(json, fields);
  // echo_map_debug(fields);

  Album a;

  if (fields.find("title") != fields.end()) {
    a.title = fields["title"];
  }
  if (fields.find("year") != fields.end()) {
    a.year = fields["year"];
  }
  // style maps to genre better than genre does, in general
  if (fields.find("styles") != fields.end()) {
    std::vector<std::string> styles;
    JsonParser::break_array(fields["styles"], styles);
    if (styles.size() > 0) {
      a.genre = styles[0];
    }
  }
  if (fields.find("artists") != fields.end()) {
    std::vector<std::string> artists;
    JsonParser::break_array(fields["artists"], artists);
    const std::vector<std::string>::size_type artists_size = artists.size();
    std::string artist = "";
    for (std::vector<std::string>::size_type i = 0; i < artists_size; ++i) {
      string_map artist_info;
      JsonParser::parse_object(artists[i], artist_info);
      if (!artist_info["anv"].empty()) {
        artist += artist_info["anv"];
      } else {
        artist += artist_info["name"];
      }
      NamingFacets::artist_facets(artist);
      if (!artist_info["join"].empty()) {
        if (artist_info["join"] != ",") {
          artist += " ";
        }
        artist += artist_info["join"];
      }
      artist += " ";
    }
    artist.erase(artist.length() - 1, 1);
    a.album_artist = artist;
  }

  if (fields.find("tracklist") != fields.end()) {
    std::vector<std::string> tracks;
    JsonParser::break_array(fields["tracklist"], tracks);
    // echo_vector_debug(tracks);

    Disc d;
    a.discs.push_back(d);
    unsigned disc = 0;
    unsigned track_num = 1;
    const std::vector<std::string>::size_type tracks_size = tracks.size();
    for (std::vector<std::string>::size_type i = 0; i < tracks_size; ++i) {
      string_map track_info;
      JsonParser::parse_object(tracks[i], track_info);
      // echo_map_debug(track_info);
      if (track_info["position"].empty()) {
        // ++disc;
        // Disc nd;
        // a.discs.push_back(nd);
        // track_num = 1;
        continue;
      }
      if (track_info["position"].find_first_of(".-") != std::string::npos &&
          string_to_numeric<unsigned>(track_info["position"].substr(
              0, track_info["position"].find_first_of(".-"))) > disc) {
        ++disc;
        Disc nd;
        a.discs.push_back(nd);
        track_num = 1;
      }
      Track t;
      t.position = track_num;
      ++track_num;
      if (track_info.find("artists") != track_info.end()) {
        std::vector<std::string> artists;
        JsonParser::break_array(track_info["artists"], artists);
        const std::vector<std::string>::size_type artists_size = artists.size();
        std::string artist = "";
        for (std::vector<std::string>::size_type i = 0; i < artists_size; ++i) {
          string_map artist_info;
          JsonParser::parse_object(artists[i], artist_info);
          if (!artist_info["anv"].empty()) {
            artist += artist_info["anv"];
          } else {
            artist += artist_info["name"];
          }
          NamingFacets::artist_facets(artist);
          if (!artist_info["join"].empty()) {
            if (artist_info["join"] != ",") {
              artist += " ";
            }
            artist += artist_info["join"];
          }
          artist += " ";
        }
        artist.erase(artist.length() - 1, 1);
        t.artist = artist;
      } else {
        t.artist = a.album_artist;
      }
      if (track_info.find("title") != track_info.end()) {
        t.title = track_info["title"];
      }
      if (track_info.find("duration") != track_info.end()) {
        std::vector<std::string> time_components;
        explode(track_info["duration"], ":", time_components);
        if (time_components.size() == 2) {
          trim(time_components[0]);
          trim(time_components[1]);
          t.length.tm_min = string_to_numeric<int>(time_components[0]);
          t.length.tm_sec = string_to_numeric<int>(time_components[1]);
        }
      }
      a.discs[disc].tracks.push_back(t);
    }
  }

  // if multidisc album, we have to remove the useless disc we created in the
  // loop
  if (a.discs.size() > 1) {
    a.discs.erase(a.discs.begin());
  }

  CueSheet cs;
  cs.album = a;
  cs.filename = filename;
  cs.comment = COMMENT;
  try {
    CueBuilder csb(cs);
  } catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    exit(1);
  }
}
}

int main(int argc, char* argv[]) {
  string first;
  if (argc < 2) {
    cout << error << endl;
    return 1;
  } else {
    first = argv[1];
  }

  if (first == "--help" || first == "-h" || first == "-H") {
    cout << help << endl;
    return 0;
  } else {
    if (argc != 3) {
      cout << error << endl;
      return 1;
    } else {
      string rel = first;
      transform(rel.begin(), rel.end(), rel.begin(), ::tolower);
      string fn(argv[2]);
      string single = rel.substr(0, 2);
      string full = rel.substr(0, 8);
      string mfull = rel.substr(0, 7);
      if (rel.find("=") == string::npos) {
        generate(rel, fn);
      } else if (single == "r=" || full == "release=") {
        generate(rel.substr(rel.find("=") + 1), fn);
      } else if (single == "m=" || mfull == "master=") {
        generate(rel.substr(rel.find("=") + 1), fn, true);
      } else {
        cout << error << endl;
        return 1;
      }
    }
  }
  return 0;
}
