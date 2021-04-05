#ifndef DCUE_HTTP_WININET_H
#define DCUE_HTTP_WININET_H

#include "http.h"

#ifdef DCUE_OFFICIAL_BUILD
#include "appkey.h"
#endif

namespace http_wininet_internal {}

// TODO
class HttpGetWinInet {
public:
  void add_header(const std::string& name, const std::string& value) {
  }
  void set_resource(const std::string& res) {
  }
  bool send(const std::string& hostname, HttpResponse& out) const {
    return false;
  }
};

#endif
