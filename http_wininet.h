#ifndef DCUE_HTTP_WININET_H
#define DCUE_HTTP_WININET_H

#include "http_types.h"

#include <memory>
#include <optional>

class HttpGetWinInet : public http_internal::HttpGetCommon {
public:
  static void global_init();
  static void global_deinit();
  std::optional<HttpResponse> send() const;
};

#endif
