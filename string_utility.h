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

#ifndef DCUE_STRING_UTILITY_H
#define DCUE_STRING_UTILITY_H

#include <sstream>
#include <string>
#include <string_view>
#include <vector>

std::string_view trim(std::string_view text);
bool replace_char(std::string& text, const char candidate,
                  const std::string& new_text);
bool replace_string(std::string& text, const std::string& candidate,
                    const std::string& new_text);

template <typename T> inline T string_to_numeric(const std::string& str) {
  std::stringstream ss(str);
  T result;
  ss >> result;
  return result;
}

template <typename T> inline std::string numeric_to_string(const T num) {
  std::stringstream ss;
  ss << num;
  return ss.str();
}

template <typename T>
inline std::string
numeric_to_padded_string(const T num, const std::string::size_type places) {
  std::string res = numeric_to_string<T>(num);
  std::string::size_type res_size = res.size();
  if (res_size < places) {
    std::string pad(places - res_size, '0');
    res = pad + res;
  }
  return res;
}

#endif
