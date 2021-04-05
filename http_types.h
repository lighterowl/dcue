#ifndef DCUE_HTTP_TYPES_H
#define DCUE_HTTP_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

enum class HttpStatus { OK, NOT_FOUND, FORBIDDEN, INT_ERR, OTHER_FAIL };

struct HttpHeader {
  std::string name;
  std::string value;
};

struct HttpResponse {
  HttpStatus status;
  std::vector<HttpHeader> headers;
  std::vector<std::uint8_t> body;
};

#endif
