#include "http_curl.h"

#include <curl/curl.h>

#include <cstdlib>
#include <iostream>

#include "defs.h"
#include "string_utility.h"

namespace {
struct CurlDeleter {
  void operator()(void* c) const {
    ::curl_easy_cleanup(c);
  }
};

size_t write_body(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto resp = static_cast<std::vector<std::uint8_t>*>(userdata);
  auto total = size * nmemb;
  resp->reserve(resp->size() + total);
  std::copy(ptr, ptr + total, std::back_inserter(*resp));
  return total;
}

size_t write_headers(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto resp = static_cast<std::vector<HttpHeader>*>(userdata);
  auto total = size * nmemb;
  HttpHeader h;
  auto hdrLine = std::string_view{ptr, total};
  auto firstColon = hdrLine.find_first_of(":");
  auto temp = hdrLine.substr(0, firstColon);
  h.name = trim(temp);
  temp = hdrLine.substr(firstColon + 1);
  h.value = trim(temp);
  resp->push_back(h);
  return total;
}
}

using CurlHandle = std::unique_ptr<void, CurlDeleter>;

void HttpGetCurl::global_init() {
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

void HttpGetCurl::global_deinit() {
  ::curl_global_cleanup();
}

void HttpGetCurl::add_header(HttpHeader&& header) {
  headers.emplace_back(header);
}

void HttpGetCurl::set_resource(const std::string& res) {
  resource = res;
}

bool HttpGetCurl::send(const std::string& hostname, HttpResponse& out) const {
  auto curl = CurlHandle{::curl_easy_init()};
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
  list = ::curl_slist_append(list, "User-Agent: " USER_AGENT);
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
