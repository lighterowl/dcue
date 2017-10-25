// *******************************************************************
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "http.h"

void HttpGet::add_header(const std::string& name, const std::string& value) {
  HttpHeader h;
  h.name = name;
  h.value = value;
  headers.push_back(h);
}

void HttpGet::set_resource(const std::string& res) {
  resource = res;
}

bool HttpGet::send(const std::string& hostname, const unsigned short port,
                   HttpResponse& out) const {
  if (resource.empty()) {
    return false;
  }
  std::string w;
  w = "GET ";
  w += resource;
  w += " " HTTP_VERSION LINE_END;

  const std::vector<HttpHeader>::size_type headers_size = headers.size();
  for (std::vector<HttpHeader>::size_type i = 0; i < headers_size; ++i) {
    w += headers[i].name;
    w += ": ";
    w += headers[i].value;
    w += LINE_END;
  }

  w += LINE_END;

  Sock s;
  if (!s.sconnect(hostname, port)) {
    return false;
  }
  if (!s.swrite(w)) {
    return false;
  }

  std::string temp;
  std::string raw_res = "";
  do {
    if (!s.sread(temp)) {
      return false;
    }
    raw_res += temp;
  } while (temp != "");

  std::string res_headers = raw_res.substr(0, raw_res.find("\r\n\r\n"));

  std::vector<std::string> lines;
  explode(res_headers, "\r\n", lines);

  std::vector<std::string> tokens;
  explode(lines[0], " ", tokens);

  // for first-line of response with format HTTP/1.1 <CODE> <DESC>
  if (tokens[1] == "200") {
    out.status = OK;
  } else if (tokens[1] == "403") {
    out.status = FORBIDDEN;
  } else if (tokens[1] == "404") {
    out.status = NOT_FOUND;
  } else if (tokens[1] == "500") {
    out.status = INT_ERR;
  } else {
    out.status = OTHER_FAIL;
  }

  if (out.status != OK) {
    return true;
  }

  const std::vector<std::string>::size_type lines_size = lines.size();
  for (std::vector<std::string>::size_type i = 1; i < lines_size; ++i) {
    HttpHeader h;
    temp = lines[i].substr(0, lines[i].find_first_of(":"));
    trim(temp);
    h.name = temp;
    temp = lines[i].substr(lines[i].find_first_of(":") + 1);
    trim(temp);
    h.value = temp;
    out.headers.push_back(h);
  }

  out.body = raw_res.substr(raw_res.find("\r\n\r\n") + 4);

  s.sdisconnect();
  return true;
}