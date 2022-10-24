// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2022 Daniel Kamil Kozar
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "multitrack_strategy.h"

namespace {
struct multitrack_strategy_single : public multitrack_strategy {
public:
  std::vector<Track>
  handle_index(const nlohmann::json& idx_track) const override {
    return convert(idx_track);
  }
  std::vector<Track> handle_medley(
      const std::vector<nlohmann::json::const_iterator>& medley_tracks)
      const override {
    return convert(*(medley_tracks[0]));
  }

  static std::vector<Track> convert(const nlohmann::json& json_trk) {
    Track t;
    t.title = json_trk.at("title");
    t.length = Track::Duration{json_trk.value("duration", std::string())};
    return {std::move(t)};
  }
};

struct multitrack_strategy_merge : public multitrack_strategy {
  std::vector<Track> handle_index(const nlohmann::json&) const override {
    return {}; // TODO
  }
  std::vector<Track> handle_medley(
      const std::vector<nlohmann::json::const_iterator>&) const override {
    return {}; // TODO
  }
};

struct multitrack_strategy_separate : public multitrack_strategy {
  std::vector<Track> handle_index(const nlohmann::json&) const override {
    return {}; // TODO
  }
  std::vector<Track> handle_medley(
      const std::vector<nlohmann::json::const_iterator>&) const override {
    return {}; // TODO
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
