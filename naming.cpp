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

#include "naming.h"

#include <cctype>

namespace {
void remove_artist_number(std::string& out) {
  const std::string::size_type out_size = out.size();
  if (out[out_size - 1] == ')' && std::isdigit(out[out_size - 2])) {
    const std::string::size_type bracket = out.find_last_of("(");
    if (bracket != std::string::npos) {
      out.erase(bracket - 1);
    }
  }
}

void reverse_artist_the(std::string& out) {
  const std::string::size_type out_size = out.size();
  if (out_size > 5 && out.substr(out_size - 5) == ", The") {
    out.erase(out_size - 5);
    out = "The " + out;
  }
}
}

void NamingFacets::artist_facets(std::string& out) {
  remove_artist_number(out);
  reverse_artist_the(out);
}
