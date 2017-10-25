// *******************************************************************
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#include "socket.h"

#include <cstring>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace {
bool hostname_to_ip(const std::string& host, std::vector<IpAddress_t>& ret) {
  // apparently gethostbyname is kinda deprecated now but it's still oh so
  // simple
  const HOSTENT* host_list = gethostbyname(host.c_str());

  // gethostbyname() failed?
  if (host_list == NULL) {
    const int last_error = get_error();
    std::cerr << "gethostbyname() failed. Error code: " << last_error
              << std::endl;
    return false;
  }

  // Check if host has at least one IP entry
  if (host_list->h_addr_list[0]) {
    for (size_t i = 0; host_list->h_addr_list[i] != 0; ++i) {
      ret.push_back((*((IpAddress_t*)host_list->h_addr_list[i])));
    }
    return true;
  }
  std::cerr << "Failed to find an IP for the host" << std::endl;
  return false;
}
}

Sock::Sock() {
  state = NONE;
  s = INVALID_SOCKET;
#ifdef _WIN
  if (!ws_init()) {
    throw std::runtime_error("Could not initialise Winsock library");
  }
#endif
  state = INIT;
}

Sock::~Sock() {
  if (state == CONNECTED) {
    sdisconnect();
  }
#ifdef _WIN
  ws_stop();
#endif
}

#ifdef _WIN
// unfortunate initialisation of winsock
bool Sock::ws_init() const {
  static bool done = false;
  if (done) {
    return true;
  }
  WSADATA ws_data;
  const int last_error = WSAStartup(WS_VERSION, &ws_data);
  if (last_error != 0) {
    std::cerr << "WSAStartup() failed. Error code: " << last_error << std::endl;
    return false;
  }
  if (ws_data.wVersion < WS_VERSION) {
    std::cerr << "Incorrect Winsock version. Wanted: < " << WS_VERSION
              << ", got: " << ws_data.wVersion << std::endl;
    return false;
  }
  done = true;

  return true;
}

bool Sock::ws_stop() const {
  static bool done = false;
  if (done) {
    return true;
  }
  if (WSACleanup() != 0) {
    const int last_error = WSAGetLastError();
    std::cerr << "WSACleanup() failed. Error code: " << last_error << std::endl;
    return false;
  }
  done = true;

  return true;
}
#endif

bool Sock::sconnect(const std::string& address, const unsigned short port) {
  s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s == INVALID_SOCKET) {
    const int last_error = get_error();
    std::cerr << "Failed to initialise socket! Error code: " << last_error
              << std::endl;
    return false;
  }

  sockaddr_in saddr;
  saddr.sin_family = AF_INET;

  std::vector<IpAddress_t> ip;
  hostname_to_ip(address, ip);

  saddr.sin_addr.s_addr = ip[0];
  saddr.sin_port = htons(port);

  if (connect(s, (sockaddr*)&saddr, (int)sizeof(saddr)) != 0) {
    const int last_error = get_error();
    std::cerr << "Socket failed to connect. Error code: " << last_error
              << std::endl;
    return false;
  }
  state = CONNECTED;
  return true;
}

bool Sock::sread(std::string& out, const size_t buffer_length) {
  if (state != CONNECTED) {
    return false;
  }
  char* recv_buffer = new char[buffer_length];

  // don't memset(), memset's probably pedantic

  const int return_val = recv(s, recv_buffer, (BUFFER_LENGTH - 1), 0);

  if (return_val == SOCKET_ERROR) {
    const int last_error = get_error();
    std::cerr << "Socket read failed. Error code: " << last_error << std::endl;
    return false;
  }
  // end of transmission (return_val == 0) will result in empty string so no
  // need to test explicitly
  recv_buffer[return_val] = '\0';
  out = recv_buffer;
  delete[] recv_buffer;
  return true;
}

bool Sock::swrite(std::string& msg, const int max_attempts) {
  if (state != CONNECTED) {
    return false;
  }

  const char* cmsg = msg.c_str();
  const size_t cmsg_size = strlen(cmsg);
  const int return_val = send(s, cmsg, cmsg_size, 0);

  if (return_val == SOCKET_ERROR) {
    const int last_error = get_error();
    std::cerr << "Socket writing failed. Error code: " << last_error
              << std::endl;
    return false;
  }
  if (max_attempts < 0) {
    std::cerr << "Exhausted all socket write attempts" << std::endl;
    return false;
  }
  if (cmsg_size > static_cast<size_t>(return_val)) {
    // Assume not all data was sent
    // provide max attempts as insurance against infinite recursion
    if (!swrite(msg.erase(0, return_val), max_attempts - 1)) {
      return false;
    }
  }
  return true;
}

bool Sock::swrite(const char* msg, const int max_attempts) {
  std::string temp(msg);
  return swrite(temp, max_attempts);
}

bool Sock::sdisconnect() {
  if (state != CONNECTED) {
    return false;
  }
  if (close(s) != 0) {
    const int last_error = get_error();
    std::cerr << "Error while closing socket. Error code: " << last_error
              << std::endl;
    return false;
  }
  state = NONE;
  return true;
}
