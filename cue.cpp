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

#include "cue.h"
#include "filename_utility.h"
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

time_t tm_to_time_t(const Tm_t& t) {
  time_t ret = 0;
  ret += t.tm_sec;
  ret += (t.tm_min * 60);
  return ret;
}

Tm_t time_t_to_tm(const time_t t) {
  Tm_t ret;
  ret.tm_min = static_cast<int>((t / 60));
  ret.tm_sec = static_cast<int>((t % 60));
  return ret;
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
  void add_index(const std::string& index, const unsigned minutes,
                 const unsigned seconds, const unsigned frames) {
    stream << "INDEX ";
    stream << index;
    stream << " ";
    add_generic_time(minutes, seconds, frames);
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
  void add_track_index(const unsigned minutes, const unsigned seconds,
                       const unsigned frames) {
    const std::string index = "01";
    add_index(index, minutes, seconds, frames);
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
}

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
    time_t cumulative = 0;
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
      Tm_t temp = time_t_to_tm(cumulative);
      c.add_track_index(temp.tm_min, temp.tm_sec, 0);
      cumulative += tm_to_time_t(track.length);
    }
  }
}
