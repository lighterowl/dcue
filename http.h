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

#ifndef DCUE_HTTP_H
#define DCUE_HTTP_H

#ifdef _WIN32
#include "http_wininet.h"
using HttpGet = HttpGetWinInet;
#else
#include "http_curl.h"
using HttpGet = HttpGetCurl;
#endif

#endif
