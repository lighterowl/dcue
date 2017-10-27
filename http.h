// *******************************************************************
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#ifndef _HTTP_H
#define _HTTP_H

#include "defs.h"
#include "string_utility.h"

#include <string>
#include <vector>
#include <memory>

enum HttpStatus_t { OK, NOT_FOUND, FORBIDDEN, INT_ERR, OTHER_FAIL };

struct HttpHeader {
  std::string name;
  std::string value;
};

struct HttpResponse {
  HttpStatus_t status;
  std::vector<HttpHeader> headers;
  std::string body;
};

struct CurlDeleter {
  void operator()(void*) const;
};

class HttpGet {
  std::vector<HttpHeader> headers;
  std::string resource;
  std::unique_ptr<void, CurlDeleter> curl;

public:
  HttpGet();
  void add_header(const std::string& name, const std::string& value);
  void set_resource(const std::string& res);
  bool send(const std::string& hostname, const unsigned short port,
            HttpResponse& out) const;
};

#endif
