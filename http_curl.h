#ifndef DCUE_HTTP_CURL_H
#define DCUE_HTTP_CURL_H

#include "http_types.h"

#include <memory>
#include <optional>

class HttpGetCurl : public http_internal::HttpGetCommon {
public:
  static void global_init();
  static void global_deinit();
  std::optional<HttpResponse> send() const;
};

#endif
