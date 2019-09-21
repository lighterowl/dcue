// *******************************************************************
// DCue (github.com/xavery/dcue)
// Copyright (c) 2019 Daniel Kamil Kozar
// Original version by :
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "http.h"
#include <cstdlib>
#include <curl/curl.h>
#include <iostream>

namespace {
struct CurlInit {
  CurlInit() {
    if (::curl_global_init(CURL_GLOBAL_DEFAULT)) {
      std::cerr << "libcurl initialisation failed\n";
      ::exit(1);
    }
    auto versinfo = ::curl_version_info(CURLVERSION_NOW);
    if (!(versinfo->features & CURL_VERSION_SSL)) {
      std::cerr << "Your libcurl doesn't support SSL\n";
      ::exit(1);
    }
  }
  ~CurlInit() {
    ::curl_global_cleanup();
  }
};

const CurlInit c;

size_t write_body(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto resp = static_cast<std::string*>(userdata);
  auto total = size * nmemb;
  resp->append(ptr, total);
  return total;
}

size_t write_headers(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto resp = static_cast<std::vector<HttpHeader>*>(userdata);
  auto total = size * nmemb;
  HttpHeader h;
  auto hdrLine = std::string{ptr, total};
  auto firstColon = hdrLine.find_first_of(":");
  auto temp = hdrLine.substr(0, firstColon);
  trim(temp);
  h.name = temp;
  temp = hdrLine.substr(firstColon + 1);
  trim(temp);
  h.value = temp;
  resp->push_back(h);
  return total;
}

HttpStatus_t rawCodeToHttpStatus(long respCode) {
  HttpStatus_t rv = OTHER_FAIL;
  switch (respCode) {
  case 200:
    rv = OK;
    break;
  case 403:
    rv = FORBIDDEN;
    break;
  case 404:
    rv = NOT_FOUND;
    break;
  case 500:
    rv = INT_ERR;
    break;
  }
  return rv;
}
}

HttpGet::HttpGet() : curl(::curl_easy_init()) {
}

void HttpGet::add_header(const std::string& name, const std::string& value) {
  HttpHeader h;
  h.name = name;
  h.value = value;
  headers.push_back(h);
}

void HttpGet::set_resource(const std::string& res) {
  resource = res;
}

bool HttpGet::send(const std::string& hostname, HttpResponse& out) const {
  if (resource.empty()) {
    return false;
  }
  auto url = hostname;
  url += resource;
  ::curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());

  struct curl_slist* list = nullptr;
  for (auto&& header : headers) {
    auto fullHeader = header.name + ": " + header.value;
    list = ::curl_slist_append(list, fullHeader.c_str());
  }
  ::curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, list);

  ::curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &out.body);
  ::curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_body);
  ::curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, &out.headers);
  ::curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, write_headers);

  ::curl_easy_perform(curl.get());
  ::curl_slist_free_all(list);

  long response_code;
  ::curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response_code);
  out.status = rawCodeToHttpStatus(response_code);

  return true;
}

void CurlDeleter::operator()(void* c) const {
  ::curl_easy_cleanup(c);
}
