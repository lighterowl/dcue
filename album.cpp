// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2022 Daniel Kamil Kozar
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "album.h"
#include "multitrack_strategy.h"
#include "naming.h"

#include <charconv>
#include <optional>
#include <stdexcept>

#include <nlohmann/json.hpp>
#include <spdlog/fmt/fmt.h>

namespace {
std::vector<std::string> tokenise_position(std::string_view position) {
  // tokenise the "position" field to something a bit more workable.
  // this does not try to interpret the meaning of each of the tokenised fields
  // since they are context-dependent, i.e. "2.5" might mean the 5th medley
  // track in the 2nd track of a single-disc release or the 5th track on the 2nd
  // disc of a multi-disc release.
  // according to Database Guidelines, multi-disc releases should use '-' as the
  // separator while medley tracks should use '.' but this is hardly enforced so
  // relying on this makes little sense.

  std::vector<std::string> rv;
  std::size_t cur_pos = 0;
  while (cur_pos < position.size()) {
    const auto sep_pos = position.find_first_of("-.", cur_pos);
    if (sep_pos != std::string::npos) {
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
get_disc_number(const std::vector<std::string>& position) {
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

Track read_track(nlohmann::json::const_iterator track, const nlohmann::json&,
                 const Album& album, const std::vector<std::string>&) {
  Track rv;
  if (track->find("artists") != track->end()) {
    rv.artist = NamingFacets::concatenate_artists((*track)["artists"]);
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
  rv.length = Track::Duration{duration};
  return rv;
}

std::vector<std::string> get_position(const nlohmann::json& entry) {
  auto position = entry.value("position", std::string{});

  if (position.empty()) {
    auto subtracks = entry.find("sub_tracks");
    if (subtracks != entry.end() && subtracks->is_array() &&
        !subtracks->empty()) {
      position = (*subtracks)[0].value("position", std::string{});
    }
  }

  if (!position.empty()) {
    return tokenise_position(position);
  } else {
    return {};
  }
}

bool is_medley(const std::vector<std::string>& position) {
  const auto last_token = position.back().back();
  return (last_token >= 'a' && last_token <= 'z') ||
         (last_token >= 'A' && last_token <= 'Z');
}

std::vector<nlohmann::json::const_iterator>
extract_medley(nlohmann::json::const_iterator current,
               nlohmann::json::const_iterator end) {
  std::vector<nlohmann::json::const_iterator> rv;
  const auto start_pos = get_position(*current);
  if (!is_medley(start_pos)) {
    return {};
  }

  rv.push_back(current);
  ++current;
  while (current != end) {
    auto cur_pos = get_position(*current);
    if (is_medley(cur_pos)) {
      rv.push_back(current);
    } else {
      break;
    }
    ++current;
  }
  return rv;
}

}

Track::Duration::Duration(std::string_view duration) {
  const auto colon_pos = duration.find(':');
  if (colon_pos == std::string_view::npos ||
      colon_pos == duration.length() - 1) {
    throw std::runtime_error(
        fmt::format("Unrecognised duration {}, qutting", duration));
  }
  auto min_conv_result =
      std::from_chars(duration.data(), duration.data() + colon_pos, min);
  auto sec_conv_result =
      std::from_chars(duration.data() + colon_pos + 1,
                      duration.data() + duration.length(), sec);
  if (min_conv_result.ec != std::errc{} || sec_conv_result.ec != std::errc{}) {
    throw std::runtime_error(
        fmt::format("Unrecognised duration {}, qutting", duration));
  }
}

Album Album::from_json(const nlohmann::json& toplevel,
                       const multitrack_strategy& index_track_strategy,
                       const multitrack_strategy& medley_track_strategy) {
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
    album.album_artist = NamingFacets::concatenate_artists(toplevel["artists"]);
  }

  const auto& tracklist = toplevel.at("tracklist");
  auto track = std::cbegin(tracklist);
  const auto tracks_end = std::cend(tracklist);

  Disc d;
  unsigned cur_disc = 1;
  while (track != tracks_end) {
    auto tokens = get_position(*track);
    if (tokens.empty()) {
      ++track;
      continue;
    }

    const auto this_disc = get_disc_number(tokens);
    if (this_disc && *this_disc > cur_disc) {
      cur_disc = *this_disc;
      album.discs.emplace_back(std::move(d));
    }

    const auto type = track->value("type_", std::string{});
    if (type == "index") {
      auto tracks = index_track_strategy.handle_index(*track, album);
      std::move(std::begin(tracks), std::end(tracks),
                std::back_inserter(d.tracks));
      ++track;
    } else if (type == "track") {
      auto medley = extract_medley(track, tracks_end);
      if (!medley.empty()) {
        auto tracks = medley_track_strategy.handle_medley(medley, album);
        std::move(std::begin(tracks), std::end(tracks),
                  std::back_inserter(d.tracks));
        track = medley.back() + 1;
      } else {
        d.tracks.emplace_back(read_track(track, tracklist, album, tokens));
        ++track;
      }
    }
  }

  album.discs.emplace_back(std::move(d));
  return album;
}
