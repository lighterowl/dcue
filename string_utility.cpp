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

#include "string_utility.h"

#include <cctype>

void explode(const std::string& text, const std::string& separator,
             std::vector<std::string>& results) {
  std::string::size_type found;
  std::string copy(text);
  const std::string::size_type separator_size = separator.length();
  do {
    found = copy.find(separator);
    if (found > 0) {
      results.push_back(copy.substr(0, found));
    }
    copy = copy.substr(found + separator_size);
  } while (found != std::string::npos);

  return;
}

void ltrim(std::string& text) {
  std::string::size_type i = 0;
  const std::string::size_type text_size = text.size();
  while (i < text_size && std::isspace(text[i])) {
    ++i;
  }
  text.erase(0, i);
}

void rtrim(std::string& text) {
  std::string::size_type i = text.size();
  while (i > 0 && std::isspace(text[i - 1])) {
    --i;
  }
  text.erase(i);
}

void trim(std::string& text) {
  ltrim(text);
  rtrim(text);
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
