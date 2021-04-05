#ifndef DCUE_HTTP_WININET_H
#define DCUE_HTTP_WININET_H

#include "http_types.h"

#include <memory>

class HttpGetWinInet {
  std::string resource;

public:
  static void global_init();
  static void global_deinit();
  void set_resource(const std::string& res);
  bool send(const std::string& hostname, HttpResponse& out) const;
};

#endif
