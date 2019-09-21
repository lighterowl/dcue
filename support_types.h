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

#ifndef _SUPPORT_TYPES_H
#define _SUPPORT_TYPES_H

#include <ctime>
#include <string>
#include <vector>

#include <unordered_map>

typedef std::tm Tm_t;

struct Track {
  std::string artist;
  std::string title;
  struct tm length;
  unsigned position;

  inline bool operator>(const Track& rhs) const {
    return this->position > rhs.position;
  }

  inline bool operator<(const Track& rhs) const {
    return this->position < rhs.position;
  }

  Track() : artist(""), title(""), position(0) {
    length.tm_min = 0;
    length.tm_sec = 0;
  }
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

struct CueSheet {
  Album album;
  std::string comment;
  std::string filename;
};

#endif
