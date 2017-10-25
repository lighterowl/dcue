// *******************************************************************
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#ifndef _DCUE_H
#define _DCUE_H

#include "defs.h"
#include "json_parser.h"
#include "discogs.h"
#include "cue.h"
#include "string_utility.h"
#include "support_types.h"
#include "naming.h"
#include "filename_utility.h"

#include <string>

class DCue {
public:
	static void generate(const std::string& id, const std::string& filename, const bool is_master = false);
};

// inline void echo_map_debug(string_map& sm) {
// 	for(string_map::iterator it = sm.begin(); it != sm.end(); ++it) {
// 		std::cout << (*it).first << "=" << (*it).second << std::endl;
// 	}
// }

// inline void echo_vector_debug(std::vector<std::string>& v) {
// 	for(size_t i = 0; i < v.size(); ++i) {
// 		std::cout << v[i] << std::endl;
// 	}
// }

#endif