// *******************************************************************
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#ifndef _JSON_PARSER_H
#define _JSON_PARSER_H

#include "defs.h"
#include "string_utility.h"

#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cstring>
#include <cctype>

#ifdef _CPP11
#include <unordered_map>
#else
#include <map>
#endif

#ifdef _CPP11
typedef std::unordered_map<std::string, std::string> string_map;
#else
typedef std::map<std::string, std::string> string_map;
#endif

class JsonParser {
	static bool is_pointless(const std::string& s);
	static void prepare_object(std::string& text);
	static void try_push_back(std::string text, std::vector<std::string>& results);
	static std::string::size_type eat_string(const std::string& cur, std::string::size_type pos);
	static std::string::size_type eat_object(const std::string& cur, std::string::size_type pos, const bool array = false);

public:
	static void parse_object(std::string text, string_map& kvs);
	static void break_string_array(std::string& text, std::vector<std::string>& results);
	static void break_object_array(std::string& text, std::vector<std::string>& results);
	static void break_array(std::string& text, std::vector<std::string>& results);
};

#endif