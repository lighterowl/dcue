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

#include "string_utility.h"

#include <cctype>

namespace {
std::string_view ltrim(std::string_view text) {
  std::string::size_type i = 0;
  const std::string::size_type text_size = text.size();
  while (i < text_size && std::isspace(text[i])) {
    ++i;
  }
  return text.substr(i);
}

std::string_view rtrim(std::string_view text) {
  std::string::size_type i = text.size();
  while (i > 0 && std::isspace(text[i - 1])) {
    --i;
  }
  return text.substr(0, i);
}
}

std::string_view trim(std::string_view text) {
  return ltrim(rtrim(text));
}

bool replace_char(std::string& text, const char candidate,
                  const std::string& new_text) {
  const std::string::size_type text_size = text.size();
  bool did_change = false;
  for (std::string::size_type i = 0; i < text_size; ++i) {
    if (text[i] == candidate) {
      text.replace(i, 1, new_text);
      did_change = true;
    }
  }
  return did_change;
}

bool replace_string(std::string& text, const std::string& candidate,
                    const std::string& new_text) {
  std::string::size_type found = text.find(candidate);
  bool did_change = false;
  while (found != std::string::npos) {
    text.replace(found, candidate.size(), new_text);
    did_change = true;
    found = text.find(candidate, found + 1);
  }
  return did_change;
}
