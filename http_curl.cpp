#include <cstdlib>
#include <iostream>

#include <curl/curl.h>

namespace {
const struct CurlInit {
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
} init;
}
