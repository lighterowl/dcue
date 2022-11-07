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

#include <string>
#include <string_view>

std::string_view trim(std::string_view text);
bool replace_char(std::string& text, const char candidate,
                  const std::string& new_text);
bool replace_string(std::string& text, const std::string& candidate,
                    const std::string& new_text);

#endif
