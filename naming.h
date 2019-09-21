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

#ifndef _NAMING_H
#define _NAMING_H

#include <string>

class NamingFacets {
  static void remove_artist_number(std::string& out);
  static void reverse_artist_the(std::string& out);

public:
  static void name_facets(std::string& out);
  static void artist_facets(std::string& out);
};

#endif
