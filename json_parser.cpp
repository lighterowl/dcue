// *******************************************************************
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "json_parser.h"

#include <cassert>

bool JsonParser::is_pointless(const std::string& s) {
  std::string::size_type s_size = s.length();
  if (s_size == 0) {
    return true;
  }
  if (s_size == 1) {
    if (std::isspace(static_cast<int>(s[0])) ||
        !std::isprint(static_cast<int>(s[0]))) {
      return true;
    }
  }
  return false;
}

void JsonParser::prepare_object(std::string& text) {
  trim(text);
  if (text[0] == '{' || text[0] == '[') {
    text.erase(0, 1);
    text.erase(text.length() - 1, 1);
  }
}

void JsonParser::try_push_back(std::string text,
                               std::vector<std::string>& results) {
  if (!is_pointless(text)) {
    trim(text);
    if (text[0] == '\"') {
      text.erase(0, 1);
      text.erase(text.length() - 1, 1);
    }
    results.push_back(text);
  }
}

std::string::size_type JsonParser::eat_string(const std::string& cur,
                                              std::string::size_type pos) {
  ++pos;
  std::string::size_type found = cur.find("\"", pos);
  while (found != std::string::npos) {
    if (cur[found - 1] != '\\') {
      pos = found + 1;
      return pos;
    }
    found = cur.find("\"", found + 1);
  }
  return std::string::npos;
}

std::string::size_type JsonParser::eat_object(const std::string& cur,
                                              std::string::size_type pos,
                                              bool array) {
  ++pos;
  int embed = 1;
  char open = (array) ? '[' : '{';
  char close = (array) ? ']' : '}';
  assert(cur.find(close, pos) != std::string::npos);
  while (embed != 0) {
    if (cur[pos] == open) {
      ++embed;
    }
    if (cur[pos] == close) {
      --embed;
    }
    ++pos;
  }
  return pos;
}

void JsonParser::parse_object(std::string text, string_map& kvs) {
  std::vector<std::string> results;
  std::string::size_type found_end = 0;

  prepare_object(text);

  std::string temp;
  std::string::size_type found = text.find_first_of(",:{[\"");
  while (found != std::string::npos) {
    temp = text.substr(0, found);
    if (text[found] == '\"') {
      try_push_back(temp, results);
      found_end = text.find_first_of(",:}]", eat_string(text, found));
      temp = text.substr(found, found_end - found);
      try_push_back(temp, results);
      found = found_end;
    } else if (text[found] == '{' || text[found] == '[') {
      try_push_back(temp, results);
      if (text[found] == '{') {
        found_end = eat_object(text, found);
      } else {
        found_end = eat_object(text, found, true);
      }
      temp = text.substr(found, found_end - found);
      try_push_back(temp, results);
      found = found_end;
    } else {
      if (!temp.empty()) {
        std::string::size_type end = temp.length() - 1;
        if (temp[end] == ',' || temp[end] == ':') {
          temp.erase(end, 1);
        }
        try_push_back(temp, results);
      }
    }
    text.erase(0, found + 1);
    if (found_end == std::string::npos) {
      found = std::string::npos;
    } else {
      found = text.find_first_of(",:{[\"");
    }
  }
  try_push_back(text, results);

  const std::vector<std::string>::size_type results_size = results.size();
  for (std::vector<std::string>::size_type i = 0; (i + 1) < results_size;
       i += 2) {
    kvs.insert(std::pair<std::string, std::string>(results[i], results[i + 1]));
  }
}

void JsonParser::break_string_array(std::string& text,
                                    std::vector<std::string>& results) {
  std::string::size_type found = text.find("\"");
  std::string::size_type found_end;
  std::string temp;
  while (found != std::string::npos) {
    found_end = eat_string(text, found);
    temp = text.substr(found + 1, found_end - found - 2);
    results.push_back(temp);
    if (found_end == std::string::npos) {
      found = std::string::npos;
    } else {
      found = text.find("\"", found_end);
    }
  }
}

void JsonParser::break_object_array(std::string& text,
                                    std::vector<std::string>& results) {
  std::string::size_type found = text.find("{");
  std::string::size_type found_end;
  std::string temp;
  while (found != std::string::npos) {
    found_end = eat_object(text, found);
    temp = text.substr(found, found_end - found);
    results.push_back(temp);
    if (found_end == std::string::npos) {
      found = std::string::npos;
    } else {
      found = text.find("{", found_end);
    }
  }
}

void JsonParser::break_array(std::string& text,
                             std::vector<std::string>& results) {
  prepare_object(text);
  trim(text);

  if (text[0] == '\"') {
    break_string_array(text, results);
  } else if (text[0] == '{') {
    break_object_array(text, results);
  }
}