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

std::vector<HttpHeader>
processRawHeaders(const std::vector<std::uint8_t>& rawHeaders) {
  std::vector<HttpHeader> rv;
  const auto end = std::end(rawHeaders);
  auto it = std::begin(rawHeaders);
  auto nextNul = std::find(it, end, 0);
  while (std::distance(it, nextNul) > 0) {
    auto colon = std::find(it, nextNul, ':');
    if (colon != nextNul) {
      std::string name(it, colon);
      it = colon;
      std::advance(it, 2);
      if (it < nextNul) {
        std::string value(it, nextNul);
        rv.push_back(HttpHeader{std::move(name), std::move(value)});
      }
    }
    it = nextNul;
    std::advance(it, 1);
    nextNul = std::find(it, end, 0);
  }
  return rv;
}
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

  std::string requestHeaders;
  for (auto&& h : headers) {
    requestHeaders.append(h.name);
    requestHeaders.append(": ");
    requestHeaders.append(h.value);
    requestHeaders.append("\x0D\x0A");
  }
#ifdef DCUE_OFFICIAL_BUILD
  {
    auto key = discogs_key::get();
    requestHeaders.append("Authorization: Discogs key=");
    requestHeaders.append(key.key);
    requestHeaders.append(", secret=");
    requestHeaders.append(key.secret);
    requestHeaders.append("\x0D\x0A");
  }
#endif

  auto handle = InetHandle{::InternetOpenUrlA(
      rootInternet, url.c_str(), requestHeaders.c_str(),
      requestHeaders.length(), INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD, 0)};
  if (handle == nullptr) {
    return false;
  }

  bool did_read_all = false;
  while (!did_read_all) {
    std::uint8_t buf[4096];
    DWORD actualRead;
    bool read_ok =
        ::InternetReadFile(handle.get(), buf, sizeof(buf), &actualRead);
    std::copy(buf, buf + actualRead, std::back_inserter(out.body));
    did_read_all = (read_ok && actualRead == 0);
  }

  DWORD httpStatus = 0;
  DWORD buflen = sizeof(httpStatus);
  ::HttpQueryInfoA(handle.get(),
                   HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &httpStatus,
                   &buflen, 0);
  out.status = rawCodeToHttpStatus(httpStatus);

  std::vector<std::uint8_t> rawHeaders;
  buflen = 0;
  ::HttpQueryInfoA(handle.get(), HTTP_QUERY_RAW_HEADERS, nullptr, &buflen, 0);
  if (GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND) {
    rawHeaders.resize(buflen);
    auto success = ::HttpQueryInfoA(handle.get(), HTTP_QUERY_RAW_HEADERS,
                                    rawHeaders.data(), &buflen, 0);
    if (success) {
      out.headers = processRawHeaders(rawHeaders);
    }
  }

  return true;
}
