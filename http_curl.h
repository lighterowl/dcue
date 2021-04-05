#ifndef DCUE_HTTP_CURL_H
#define DCUE_HTTP_CURL_H

#include "http_types.h"

#include <memory>

class HttpGetCurl {
  std::vector<HttpHeader> headers;
  std::string resource;

public:
  static void global_init();
  static void global_deinit();
  void add_header(const std::string& name, const std::string& value);
  void set_resource(const std::string& res);
  bool send(const std::string& hostname, HttpResponse& out) const;
};

#endif
