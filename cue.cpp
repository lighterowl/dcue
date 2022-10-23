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

#include "cue.h"

#include "defs.h"
#include "naming.h"
#include "string_utility.h"
#include "support_types.h"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <optional>

#include <spdlog/fmt/fmt.h>

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

void open_file(std::ofstream& out, const std::filesystem::path& fpath,
               unsigned disc = 0) {
  auto cuepath = fpath;
  if (disc != 0) {
    const auto disc_str = std::to_string(disc);
    auto fname = cuepath.stem().string();
    if (!replace_char(fname, '?', disc_str)) {
      cuepath.replace_filename(fname + '-' + disc_str);
    }
  }

  cuepath.replace_extension(".cue");

  /* the output stream that the contents of the CUE are written into is opened
   * in binary mode in order to preserve CRLF line endings. */
  out.open(cuepath, std::ios::binary | std::ios::out);
  if (!out.is_open()) {
    throw std::runtime_error(
        fmt::format("Cannot open output file! (\"{}\")", cuepath.string()));
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
      auto join = artist_info.value("join", ",");
      if (join != ",") {
        rv += " ";
      }
      rv += join;
      rv += " ";
    }
  }
  return rv;
}

std::vector<std::string_view> tokenise_position(std::string_view position) {
  // tokenise the "position" field to something a bit more workable.
  // this does not try to interpret the meaning of each of the tokenised fields
  // since they are context-dependent, i.e. "2.5" might mean the 5th medley
  // track in the 2nd track of a single-disc release or the 5th track on the 2nd
  // disc of a multi-disc release.
  // according to Database Guidelines, multi-disc releases should use '-' as the
  // separator while medley tracks should use '.' but this is hardly enforced so
  // relying on this makes little sense.

  std::vector<std::string_view> rv;
  std::size_t cur_pos = 0;
  while (cur_pos < position.size()) {
    const auto sep_pos = position.find_first_of("-.", cur_pos);
    if (sep_pos != std::string_view::npos) {
      // up to but not including the separator
      rv.emplace_back(position.substr(cur_pos, sep_pos - cur_pos));
      cur_pos = sep_pos + 1;
    } else {
      // all that's left
      rv.emplace_back(position.substr(cur_pos));
      break;
    }
  }
  return rv;
}

std::optional<unsigned>
get_disc_number(const std::vector<std::string_view>& position) {
  if (position.size() < 2) {
    return std::nullopt;
  }
  auto pre_dot = position[0];
  auto post_dot = position[1];
  unsigned discno;
  auto pre_dot_result =
      std::from_chars(pre_dot.data(), pre_dot.data() + pre_dot.size(), discno);
  unsigned dummy;
  auto post_dot_result = std::from_chars(
      post_dot.data(), post_dot.data() + post_dot.size(), dummy);
  if (pre_dot_result.ec == std::errc{} && post_dot_result.ec == std::errc{}) {
    return discno;
  } else {
    return std::nullopt;
  }
}

class Cue {
  std::ostream& stream;

  void add_meta(const std::string& comment) {
    stream << "REM " << comment << "\r\n";
  }
  void add_generic_time(const unsigned minutes, const unsigned seconds,
                        const unsigned frames) {
    stream << std::setfill('0') << std::setw(2) << minutes << ':'
           << std::setfill('0') << std::setw(2) << seconds << ':'
           << std::setfill('0') << std::setw(2) << frames;
  }
  void add_index(const char* index, unsigned minutes, unsigned seconds) {
    stream << "INDEX " << index << ' ';
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
    stream << "PERFORMER " << std::quoted(artist) << "\r\n";
  }
  void add_title(const std::string& title) {
    stream << "TITLE " << std::quoted(title) << "\r\n";
  }
  void add_track(const unsigned num) {
    stream << "TRACK " << std::setfill('0') << std::setw(2) << num
           << " AUDIO\r\n";
  }
  void add_track_index(const unsigned minutes, const unsigned seconds) {
    add_index("01", minutes, seconds);
  }
  void add_filename(const std::string& name) {
    std::string t = name.substr(name.find_last_of(".") + 1);
    stream << "FILE " << std::quoted(name) << ' ';
    add_type_from_ext(t);
    stream << "\r\n";
  }
  void add_indent() {
    stream << '\t';
  }
  Cue(std::ostream& os) : stream(os) {
  }
};

void Cue_build(const Album& album, const std::filesystem::path& fpath) {
  for (auto i = 0u; i < album.discs.size(); ++i) {
    auto&& disc = album.discs[i];
    auto discno = i + 1;
    std::ofstream f;
    if (album.discs.size() > 1) {
      open_file(f, fpath, discno);
    } else {
      open_file(f, fpath);
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
    if (!fpath.empty()) {
      auto fname = fpath.filename().string();
      replace_char(fname, '?', std::to_string(discno));
      c.add_filename(fname);
    }
    Track::Duration total;
    for (unsigned int i = 0; i < disc.tracks.size(); ++i) {
      auto& track = disc.tracks[i];
      c.add_indent();
      c.add_track(i + 1);
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

Track::Duration parse_duration(std::string_view dur) {
  const auto colon_pos = dur.find(':');
  if (colon_pos == std::string_view::npos || colon_pos == dur.length() - 1) {
    throw std::runtime_error(
        fmt::format("Unrecognised duration {}, qutting", dur));
  }
  unsigned min, sec;
  auto min_conv_result =
      std::from_chars(dur.data(), dur.data() + colon_pos, min);
  auto sec_conv_result = std::from_chars(dur.data() + colon_pos + 1,
                                         dur.data() + dur.length(), sec);
  if (min_conv_result.ec == std::errc{} && sec_conv_result.ec == std::errc{}) {
    return Track::Duration{min, sec};
  } else {
    throw std::runtime_error(
        fmt::format("Unrecognised duration {}, qutting", dur));
  }
}

Track read_track(nlohmann::json::const_iterator track,
                 const nlohmann::json& , const Album& album,
                 const std::vector<std::string_view>& ) {
  Track rv;
  if (track->find("artists") != track->end()) {
    rv.artist = concatenate_artists((*track)["artists"]);
  } else {
    rv.artist = album.album_artist;
  }
  rv.title = track->value("title", std::string());
  auto duration = track->value("duration", std::string());
  if (duration.empty()) {
    throw std::runtime_error(
        fmt::format("Track {} has no duration : quitting",
                    track->value("position", std::string())));
  }
  rv.length = parse_duration(duration);
  return rv;
}
}

void generate(const nlohmann::json& toplevel,
              const std::filesystem::path& fpath) {
  Album album;
  album.title = toplevel.value("title", std::string());
  auto year = toplevel.value("year", -1);
  if (year != -1) {
    album.year = std::to_string(year);
  }
  // style maps to genre better than genre does, in general
  if (toplevel.find("styles") != toplevel.end()) {
    album.genre = toplevel["styles"][0].get<std::string>();
  }
  if (toplevel.find("artists") != toplevel.end()) {
    album.album_artist = concatenate_artists(toplevel["artists"]);
  }

  Disc d;
  album.discs.push_back(d);
  unsigned disc = 0;

  const auto& tracklist = toplevel.at("tracklist");
  for (auto track = std::begin(tracklist); track != std::end(tracklist);
       ++track) {
    auto position = track->value("position", std::string());
    if (position.empty()) {
      continue;
    }
    auto tokens = tokenise_position(position);
    const auto this_disc = get_disc_number(tokens);
    if (this_disc && *this_disc > disc) {
      ++disc;
      Disc nd;
      album.discs.push_back(nd);
    }

    album.discs[disc].tracks.emplace_back(
        read_track(track, tracklist, album, tokens));
  }

  // if multidisc album, we have to remove the useless disc we created in the
  // loop
  if (album.discs.size() > 1) {
    album.discs.erase(album.discs.begin());
  }

  Cue_build(album, fpath);
}
