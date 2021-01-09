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

#include "cue.h"
#include "filename_utility.h"
#include "naming.h"
#include "string_utility.h"
#include "support_types.h"

#include <algorithm>
#include <fstream>

namespace {
std::string sanitise_string(const std::string& str) {
  auto out = str;
  replace_string(out, "\\u2019", "\'");
  replace_string(out, "\\u2026", "...");
  replace_string(out, "\\\"", "\'");
  replace_char(out, '\\', "-");
  replace_char(out, '/', "-");
  return out;
}

void open_file(std::ofstream& out, const std::string& filename,
               unsigned disc = 0) {
  auto cuepath = path(filename) + basename(filename);

  if (disc != 0) {
    if (!replace_char(cuepath, '?', numeric_to_string<unsigned>(disc))) {
      cuepath += "-" + numeric_to_string<unsigned>(disc);
    }
  }

  cuepath += ".cue";

  /* the output stream that the contents of the CUE are written into is opened
   * in binary mode in order to preserve CRLF line endings. */
  out.open(cuepath, std::ios::binary | std::ios::out);
  if (!out.is_open()) {
    throw std::runtime_error("Cannot open output file! (\"" + filename + "\")");
  }
}

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

class Cue {
  std::ostream& stream;

  void add_meta(const std::string& comment) {
    stream << "REM ";
    stream << comment;
    stream << "\r\n";
  }
  void add_generic_time(const unsigned minutes, const unsigned seconds,
                        const unsigned frames) {
    stream << numeric_to_padded_string<unsigned>(minutes, 2);
    stream << ":";
    stream << numeric_to_padded_string<unsigned>(seconds, 2);
    stream << ":";
    stream << numeric_to_padded_string<unsigned>(frames, 2);
  }
  void add_index(const char* index, unsigned minutes, unsigned seconds) {
    stream << "INDEX ";
    stream << index;
    stream << " ";
    /* Discogs timestamps aren't detailed enough to provide CD frame accuracy,
     * so we just go with 0 here. */
    add_generic_time(minutes, seconds, 0);
    stream << "\r\n";
  }
  void add_type_from_ext(const std::string& ext) {
    std::string extension = ext;
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   ::tolower);
    if (extension == "mp3") {
      stream << "MP3";
    } else if (extension == "aiff") {
      stream << "AIFF";
    } else {
      stream << "WAVE";
    }
  }

public:
  void add_genre(const std::string& genre) {
    add_meta("GENRE " + genre);
  }
  void add_year(const std::string& year) {
    add_meta("DATE " + year);
  }
  void add_comment(const std::string& comment) {
    add_meta("COMMENT \"" + comment + "\"");
  }
  void add_artist(const std::string& artist) {
    stream << "PERFORMER \"";
    stream << artist;
    stream << "\"";
    stream << "\r\n";
  }
  void add_title(const std::string& title) {
    stream << "TITLE \"";
    stream << title;
    stream << "\"";
    stream << "\r\n";
  }
  void add_track(const unsigned num) {
    stream << "TRACK ";
    stream << numeric_to_padded_string<unsigned>(num, 2);
    stream << " AUDIO";
    stream << "\r\n";
  }
  void add_track_index(const unsigned minutes, const unsigned seconds) {
    add_index("01", minutes, seconds);
  }
  void add_filename(const std::string& name) {
    std::string t = name.substr(name.find_last_of(".") + 1);
    stream << "FILE \"";
    stream << name;
    stream << "\" ";
    add_type_from_ext(t);
    stream << "\r\n";
  }
  void add_indent() {
    stream << "\t";
  }
  Cue(std::ostream& os) : stream(os) {
  }
};

void Cue_build(const Album& album, const std::string& filename) {
  for (auto i = 0u; i < album.discs.size(); ++i) {
    auto&& disc = album.discs[i];
    auto discno = i + 1;
    std::ofstream f;
    if (album.discs.size() > 1) {
      open_file(f, filename, discno);
    } else {
      open_file(f, filename);
    }
    Cue c(f);
    // string sanitisation is really just a way of compensating for the number
    // of dumb cue tools available, double quotes especially confuse them and
    // Medieval CUE Splitter on Windows practically blows up when confronted
    // with backslashes in titles  it's not really the application's job to do
    // this (bar perhaps the double quotes) but because there's no actual
    // standard for cue sheets we have to make do and mend
    if (!album.genre.empty()) {
      c.add_genre(sanitise_string(album.genre));
    }
    if (!album.year.empty()) {
      c.add_year(album.year);
    }
    c.add_comment(COMMENT);
    if (!album.album_artist.empty()) {
      c.add_artist(sanitise_string(album.album_artist));
    }
    if (!album.title.empty()) {
      c.add_title(sanitise_string(album.title));
    }
    if (!filename.empty()) {
      auto fn = basename(filename) + "." + extension(filename);
      replace_char(fn, '?', numeric_to_string<unsigned>(discno));
      c.add_filename(fn);
    }
    Track::Duration total;
    for (auto&& track : disc.tracks) {
      c.add_indent();
      c.add_track(track.position);
      c.add_indent();
      c.add_indent();
      if (!track.title.empty()) {
        c.add_title(sanitise_string(track.title));
      }
      c.add_indent();
      c.add_indent();
      if (!track.artist.empty()) {
        c.add_artist(sanitise_string(track.artist));
      }
      c.add_indent();
      c.add_indent();
      c.add_track_index(total.min, total.sec);
      total += track.length;
    }
  }
}

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
      std::stringstream ss;
      ss << "Track " << track_num << ", disc " << disc
         << " has no duration, quitting\n";
      throw std::runtime_error(ss.str());
    }
    std::vector<std::string> time_components;
    explode(duration, ":", time_components);
    if (time_components.size() == 2) {
      trim(time_components[0]);
      trim(time_components[1]);
      t.length.min = string_to_numeric<unsigned>(time_components[0]);
      t.length.sec = string_to_numeric<unsigned>(time_components[1]);
    } else {
      std::stringstream ss;
      ss << "Unrecognised duration " << duration << ", quitting\n";
      throw std::runtime_error(ss.str());
    }
    ++track_num;
    a.discs[disc].tracks.push_back(t);
  }

  // if multidisc album, we have to remove the useless disc we created in the
  // loop
  if (a.discs.size() > 1) {
    a.discs.erase(a.discs.begin());
  }

  Cue_build(a, filename);
}
