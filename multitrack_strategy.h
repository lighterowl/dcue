// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2022 Daniel Kamil Kozar
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#ifndef DCUE_MULTITRACK_STRATEGY_H
#define DCUE_MULTITRACK_STRATEGY_H

#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

#include "album.h"

// implements desired behaviour when handling index or medley tracks
struct multitrack_strategy {
  virtual std::vector<Track>
  handle_index(const nlohmann::json& idx_track) const = 0;

  virtual std::vector<Track> handle_medley(
      const std::vector<nlohmann::json::const_iterator>& medley_tracks)
      const = 0;

  static std::unique_ptr<multitrack_strategy> single();
  static std::unique_ptr<multitrack_strategy> merge();
  static std::unique_ptr<multitrack_strategy> separate();
};

#endif
