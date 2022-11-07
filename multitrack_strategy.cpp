// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2022 Daniel Kamil Kozar
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "multitrack_strategy.h"

#include "naming.h"

#include <spdlog/fmt/fmt.h>

namespace {
struct multitrack_strategy_single : public multitrack_strategy {
public:
  std::vector<Track> handle_index(const nlohmann::json& idx_track,
                                  const Album& album) const override {
    return convert(idx_track, album);
  }
  std::vector<Track> handle_medley(
      const std::vector<nlohmann::json::const_iterator>& medley_tracks,
      const Album& album) const override {
    return convert(*(medley_tracks[0]), album);
  }

  static std::vector<Track> convert(const nlohmann::json& json_trk,
                                    const Album& album) {
    Track t;
    t.title = json_trk.at("title");
    t.length = Track::Duration{json_trk.value("duration", std::string())};
    auto artists = json_trk.find("artists");
    if (artists != json_trk.end()) {
      t.artist = NamingFacets::concatenate_artists(json_trk["artists"]);
    } else {
      t.artist = album.album_artist;
    }
    return {std::move(t)};
  }
};

struct multitrack_strategy_merge : public multitrack_strategy {
  std::vector<Track> handle_index(const nlohmann::json& idx_track,
                                  const Album& album) const override {
    auto& subtracks = idx_track["sub_tracks"];
    if (subtracks.empty()) {
      return {};
    }

    Track rv;
    rv.length = Track::Duration{idx_track.value("duration", std::string())};
    rv.title = idx_track.value("title", std::string());
    rv.title += " : ";
    for (auto& sub : subtracks) {
      if (sub.value("type_", std::string()) == "track") {
        rv.title += sub.value("title", std::string());
        if (&sub != &subtracks.back()) {
          rv.title += " / ";
        }
      }
    }
    rv.artist = album.album_artist;
    return {rv};
  }

  std::vector<Track> handle_medley(
      const std::vector<nlohmann::json::const_iterator>& medley_tracks,
      const Album& album) const override {
    Track rv;
    rv.length = Track::Duration{
        medley_tracks.front()->value("duration", std::string())};
    for (auto& t : medley_tracks) {
      rv.title += t->value("title", std::string());
      auto artists = t->find("artists");
      if (artists != t->end()) {
        rv.artist += NamingFacets::concatenate_artists(*artists);
      }
      if (&t != &medley_tracks.back()) {
        rv.title += " / ";
        rv.artist += " / ";
      }
    }
    if (rv.artist.empty()) {
      rv.artist = album.album_artist;
    }
    return {rv};
  }
};

struct multitrack_strategy_separate : public multitrack_strategy {
  std::vector<Track> handle_index(const nlohmann::json& idx_track,
                                  const Album& album) const override {
    auto rv = std::vector<Track>{};
    for (auto& subtrack : idx_track["sub_tracks"]) {
      Track t;
      t.artist = album.album_artist;
      t.title = fmt::format("{} : {}", idx_track.value("title", std::string()),
                            subtrack.value("title", std::string()));
      t.length = Track::Duration{subtrack.value("duration", std::string())};
      rv.emplace_back(std::move(t));
    }
    return rv;
  }
  std::vector<Track>
  handle_medley(const std::vector<nlohmann::json::const_iterator>& tracks,
                const Album&) const override {
    auto rv = std::vector<Track>{};
    for (auto& t : tracks) {
      Track parsed_track;
      parsed_track.artist = NamingFacets::concatenate_artists((*t)["artists"]);
      parsed_track.title = t->value("title", std::string());
      parsed_track.length =
          Track::Duration{t->value("duration", std::string())};
      rv.emplace_back(std::move(parsed_track));
    }
    return rv;
  }
};
}

std::unique_ptr<multitrack_strategy> multitrack_strategy::single() {
  return std::make_unique<multitrack_strategy_single>();
}

std::unique_ptr<multitrack_strategy> multitrack_strategy::merge() {
  return std::make_unique<multitrack_strategy_merge>();
}

std::unique_ptr<multitrack_strategy> multitrack_strategy::separate() {
  return std::make_unique<multitrack_strategy_separate>();
}
