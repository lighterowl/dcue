// *******************************************************************
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

void Cue::add_meta(const std::string& comment) {
  out += "REM ";
  out += comment;
  out += LINE_END;
}

void Cue::add_generic_time(const unsigned minutes, const unsigned seconds,
                           const unsigned frames) {
  out += numeric_to_padded_string<unsigned>(minutes, 2);
  out += ":";
  out += numeric_to_padded_string<unsigned>(seconds, 2);
  out += ":";
  out += numeric_to_padded_string<unsigned>(frames, 2);
}

void Cue::add_index(const std::string& index, const unsigned minutes,
                    const unsigned seconds, const unsigned frames) {
  out += "INDEX ";
  out += index;
  out += " ";
  add_generic_time(minutes, seconds, frames);
  out += LINE_END;
}

void Cue::add_type_from_ext(const std::string& ext) {
  std::string extension = ext;
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 ::tolower);
  if (extension == "mp3") {
    out += "MP3";
  } else if (extension == "aiff") {
    out += "AIFF";
  } else {
    out += "WAVE";
  }
}

void Cue::add_genre(const std::string& genre) {
  add_meta("GENRE " + genre);
}

void Cue::add_year(const std::string& year) {
  add_meta("DATE " + year);
}

void Cue::add_comment(const std::string& comment) {
  add_meta("COMMENT \"" + comment + "\"");
}

void Cue::add_artist(const std::string& artist) {
  out += "PERFORMER \"";
  out += artist;
  out += "\"";
  out += LINE_END;
}

void Cue::add_title(const std::string& title) {
  out += "TITLE \"";
  out += title;
  out += "\"";
  out += LINE_END;
}

void Cue::add_track(const unsigned num) {
  out += "TRACK ";
  out += numeric_to_padded_string<unsigned>(num, 2);
  out += " AUDIO";
  out += LINE_END;
}

void Cue::add_track_index(const unsigned minutes, const unsigned seconds,
                          const unsigned frames) {
  const std::string index = "01";
  add_index(index, minutes, seconds, frames);
}

void Cue::add_filename(const std::string& name) {
  std::string t = name.substr(name.find_last_of(".") + 1);
  out += "FILE \"";
  out += name;
  out += "\" ";
  add_type_from_ext(t);
  out += LINE_END;
}

void Cue::add_indent() {
  out += "\t";
}

const std::string& Cue::get_output() const {
  return out;
}

void CueBuilder::build() {
  std::string fn;
  const std::vector<Disc>::size_type discs_size = cuesheet.album.discs.size();
  for (std::vector<Disc>::size_type i = 0; i < discs_size; ++i) {
    // string sanitisation is really just a way of compensating for the number
    // of dumb cue tools available, double quotes especially confuse them and
    // Medieval CUE Splitter on Windows practically blows up when confronted
    // with backslashes in titles  it's not really the application's job to do
    // this (bar perhaps the double quotes) but because there's no actual
    // standard for cue sheets we have to make do and mend
    Cue c;
    if (!cuesheet.album.genre.empty()) {
      c.add_genre(sanitise_string(cuesheet.album.genre));
    }
    if (!cuesheet.album.year.empty()) {
      c.add_year(cuesheet.album.year);
    }
    if (!cuesheet.comment.empty()) {
      c.add_comment(cuesheet.comment);
    }
    if (!cuesheet.album.album_artist.empty()) {
      c.add_artist(sanitise_string(cuesheet.album.album_artist));
    }
    if (!cuesheet.album.title.empty()) {
      c.add_title(sanitise_string(cuesheet.album.title));
    }
    if (!cuesheet.filename.empty()) {
      fn = basename(cuesheet.filename) + "." + extension(cuesheet.filename);
      replace_char(fn, '?', numeric_to_string<unsigned>(i + 1));
      c.add_filename(fn);
    }
    time_t cumulative = 0;
    const std::vector<Track>::size_type tracks_size =
        cuesheet.album.discs[i].tracks.size();
    for (std::vector<Track>::size_type j = 0; j < tracks_size; ++j) {
      c.add_indent();
      c.add_track(cuesheet.album.discs[i].tracks[j].position);
      c.add_indent();
      c.add_indent();
      if (!cuesheet.album.discs[i].tracks[j].title.empty()) {
        c.add_title(sanitise_string(cuesheet.album.discs[i].tracks[j].title));
      }
      c.add_indent();
      c.add_indent();
      if (!cuesheet.album.discs[i].tracks[j].artist.empty()) {
        c.add_artist(sanitise_string(cuesheet.album.discs[i].tracks[j].artist));
      }
      c.add_indent();
      c.add_indent();
      Tm_t temp = time_t_to_tm(cumulative);
      c.add_track_index(temp.tm_min, temp.tm_sec, 0);
      cumulative += tm_to_time_t(cuesheet.album.discs[i].tracks[j].length);
    }
    if (discs_size > 1) {
      write_file(c, i + 1);
    } else {
      write_file(c);
    }
  }
}

void CueBuilder::write_file(const Cue& c, const unsigned disc) {
  std::string filename = path(cuesheet.filename) + basename(cuesheet.filename);

  if (disc != 0) {
    if (!replace_char(filename, '?', numeric_to_string<unsigned>(disc))) {
      filename += "-" + numeric_to_string<unsigned>(disc);
    }
  }

  filename += ".cue";

  std::ofstream out;
  out.open(filename, std::ios::binary | std::ios::out);
  if (!out.is_open()) {
    throw std::runtime_error("Cannot open output file! (\"" + filename + "\")");
  }
  out.write(c.get_output().c_str(), c.get_output().length());
  out.close();
}
