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

#ifndef DCUE_ALBUM_H
#define DCUE_ALBUM_H

#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json_fwd.hpp>

struct Track {
  std::string artist;
  std::string title;
  struct Duration {
    Duration() = default;
    Duration(std::string_view);
    unsigned min = 0;
    unsigned sec = 0;
    Duration& operator+=(const Duration& d) {
      sec += d.sec;
      min += d.min + (sec / 60);
      sec %= 60;
      return *this;
    }
  } length;
};

struct Disc {
  std::vector<Track> tracks;
};

struct Album {
  std::vector<Disc> discs;
  std::string year;
  std::string genre;
  std::string title;
  std::string album_artist;
  Album from_json(const nlohmann::json& toplevel);
};

#endif
