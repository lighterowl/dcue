// *******************************************************************
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#ifndef _DEFS_H
#define _DEFS_H

// should be a pretty exhaustive check for Windows
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) ||                \
    defined(__MINGW32__) || defined(__BORLANDC__) || defined(_WIN64)
#define _WIN
#endif

#define VERSION "1.0"
#define USER_AGENT "DCue/" VERSION
#define HTTP_VERSION "HTTP/1.1"
#define COMMENT "DCue v" VERSION

// can probably be changed to "\n" on linux but there's no actual need to
#define LINE_END "\r\n"

#endif
