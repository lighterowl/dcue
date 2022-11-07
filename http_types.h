#ifndef DCUE_HTTP_TYPES_H
#define DCUE_HTTP_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

enum class HttpStatus { OK, NOT_FOUND, FORBIDDEN, INT_ERR, OTHER_FAIL };

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

struct HttpHeader {
  std::string name;
  std::string value;
};

struct HttpResponse {
  HttpStatus status;
  std::vector<HttpHeader> headers;
  std::vector<std::uint8_t> body;
};

namespace http_internal {
class HttpGetCommon {
protected:
  std::vector<HttpHeader> headers;
  std::string hostname;
  std::string resource;

public:
  void add_header(HttpHeader&& header) {
    headers.emplace_back(header);
  }
  void set_resource(std::string_view res) {
    resource = res;
  }
  void set_hostname(std::string_view hname) {
    hostname = hname;
  }
};
}

#endif
