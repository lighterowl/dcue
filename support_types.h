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

#ifndef DCUE_SUPPORT_TYPES_H
#define DCUE_SUPPORT_TYPES_H

#include <string>
#include <vector>

struct Track {
  std::string artist;
  std::string title;
  struct Duration {
    unsigned min = 0;
    unsigned sec = 0;
    void operator+=(const Duration& d) {
      sec += d.sec;
      min += d.min + (sec / 60);
      sec %= 60;
    }
  } length;
  unsigned position = 0;
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
};

#endif
