#ifndef DCUE_HTTP_CURL_H
#define DCUE_HTTP_CURL_H

#include "http.h"

#include <curl/curl.h>

#ifdef DCUE_OFFICIAL_BUILD
#include "appkey.h"
#endif

namespace http_curl_internal {

struct CurlDeleter {
  void operator()(void* c) const {
    ::curl_easy_cleanup(c);
  }
};

inline size_t write_body(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto resp = static_cast<std::vector<std::uint8_t>*>(userdata);
  auto total = size * nmemb;
  resp->reserve(resp->size() + total);
  std::copy(ptr, ptr + total, std::back_inserter(*resp));
  return total;
}

inline size_t write_headers(char* ptr, size_t size, size_t nmemb,
                            void* userdata) {
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

inline HttpStatus rawCodeToHttpStatus(long respCode) {
  auto rv = HttpStatus::OTHER_FAIL;
  switch (respCode) {
  case 200:
    rv = HttpStatus::OK;
    break;
  case 403:
    rv = HttpStatus::FORBIDDEN;
    break;
  case 404:
    rv = HttpStatus::NOT_FOUND;
    break;
  case 500:
    rv = HttpStatus::INT_ERR;
    break;
  }
  return rv;
}
}

class HttpGetCurl {
  std::vector<HttpHeader> headers;
  std::string resource;
  std::unique_ptr<void, http_curl_internal::CurlDeleter> curl;

public:
  HttpGetCurl() : curl(::curl_easy_init()) {
  }
  void add_header(const std::string& name, const std::string& value) {
    HttpHeader h;
    h.name = name;
    h.value = value;
    headers.push_back(h);
  }
  void set_resource(const std::string& res) {
    resource = res;
  }
  bool send(const std::string& hostname, HttpResponse& out) const {
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
#ifdef DCUE_OFFICIAL_BUILD
    {
      auto key = discogs_key::get();
      std::string auth_hdr("Authorization: Discogs key=");
      auth_hdr.append(key.key);
      auth_hdr.append(", secret=");
      auth_hdr.append(key.secret);
      list = ::curl_slist_append(list, auth_hdr.c_str());
    }
#endif
    ::curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, list);

    ::curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &out.body);
    ::curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION,
                       http_curl_internal::write_body);
    ::curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, &out.headers);
    ::curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION,
                       http_curl_internal::write_headers);

    ::curl_easy_perform(curl.get());
    ::curl_slist_free_all(list);

    long response_code;
    ::curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response_code);
    out.status = http_curl_internal::rawCodeToHttpStatus(response_code);

    return true;
  }
};

#endif
