#include "http_wininet.h"
#include "defs.h"

#include <iostream>

#include <Windows.h>

#include <WinInet.h>

#ifdef DCUE_OFFICIAL_BUILD
#include "appkey.h"
#endif

namespace {
HINTERNET rootInternet;

struct InternetHandleDeleter {
  void operator()(void* h) const {
    ::InternetCloseHandle(h);
  }
};
}

using InetHandle = std::unique_ptr<void, InternetHandleDeleter>;

void HttpGetWinInet::global_init() {
  rootInternet = ::InternetOpenA(USER_AGENT, INTERNET_OPEN_TYPE_PRECONFIG,
                                 nullptr, nullptr, 0);
  if (rootInternet == nullptr) {
    DWORD err = ::GetLastError();
    std::cerr << "Creating root internet handle failed (GetLastError = " << err
              << '\n';
    ::exit(1);
  }
}

void HttpGetWinInet::global_deinit() {
  ::InternetCloseHandle(rootInternet);
}

void HttpGetWinInet::set_resource(const std::string& res) {
  resource = res;
}

bool HttpGetWinInet::send(const std::string& hostname,
                          HttpResponse& out) const {
  if (resource.empty()) {
    return false;
  }

  auto url = hostname;
  url += resource;

  std::string headers;
#ifdef DCUE_OFFICIAL_BUILD
  {
    auto key = discogs_key::get();
    headers.append("Authorization: Discogs key=");
    headers.append(key.key);
    headers.append(", secret=");
    headers.append(key.secret);
    headers.append("\x0D\x0A");
  }
#endif

  auto handle = InetHandle{::InternetOpenUrlA(
      rootInternet, url.c_str(), headers.c_str(), headers.length(),
      INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD, 0)};
  if (handle == nullptr) {
    return false;
  }

  DWORD actualRead;
  bool read_all = false;
  while (!read_all) {
    std::uint8_t buf[4096];
    bool read_ok =
        ::InternetReadFile(handle.get(), buf, sizeof(buf), &actualRead);
    std::copy(buf, buf + actualRead, std::back_inserter(out.body));
    read_all = (read_ok && actualRead == 0);
  }

  DWORD httpStatus = 0;
  DWORD buflen = sizeof(httpStatus);
  ::HttpQueryInfoA(handle.get(),
                   HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &httpStatus,
                   &buflen, 0);
  out.status = rawCodeToHttpStatus(httpStatus);

  return true;
}
