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

#include "filename_utility.h"

std::string basename(const std::string& fn) {
  const std::string::size_type pos = fn.find_last_of("\\/") + 1;
  std::string basename;
  if (pos != std::string::npos) {
    basename = fn.substr(pos, fn.find_last_of(".") - pos);
  } else {
    basename = fn.substr(0, fn.find_last_of("."));
  }
  return basename;
}

std::string extension(const std::string& fn) {
  return fn.substr(fn.find_last_of(".") + 1);
}

std::string path(const std::string& fn) {
  const std::string::size_type pos = fn.find_last_of("\\/") + 1;
  if (pos != std::string::npos) {
    return fn.substr(0, pos);
  } else {
    return "";
  }
}
