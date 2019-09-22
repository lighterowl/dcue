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

#ifndef DCUE_FILENAME_UTILITY_H
#define DCUE_FILENAME_UTILITY_H

#include <string>

std::string basename(const std::string& fn);
std::string extension(const std::string& fn);
std::string path(const std::string& fn);

#endif
