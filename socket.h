// *******************************************************************
// DCue (sourceforge.net/projects/dcue)
// Copyright (c) 2013 Fluxtion, DCue project
// *******************************************************************
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// *******************************************************************

#ifndef _SOCKET_H
#define _SOCKET_H

#include "defs.h"

#include <string>
#include <vector>

#ifdef _WIN
#define _WINSOCKAPI_
#include <windows.h>
#include <windef.h>
#include <winsock2.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cerrno>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#endif

#ifndef _WIN
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef struct hostent HOSTENT;
typedef int SOCKET;
#else
//version 2.0
#define WS_VERSION 0x02
#define close closesocket
#endif

namespace {
inline int get_error() {
#ifdef _WIN
	return WSAGetLastError();
#else
	return errno;
#endif
}
}

typedef unsigned long IpAddress_t;

enum SocketState_t { NONE, INIT, CONNECTED };

class Sock {
	const static size_t BUFFER_LENGTH = 1024;
	const static int MAX_ATTEMPTS = 20;
	SOCKET s;
	SocketState_t state;

#ifdef _WIN
	//unfortunate initialisation of winsock
	bool ws_init() const;
	bool ws_stop() const;
#endif

public:
	Sock();
	~Sock();

	bool sconnect(const std::string& address, const unsigned short port);
	bool sread(std::string& out, const size_t buffer_length = BUFFER_LENGTH);
	bool sread_to_end(std::string& out);
	bool swrite(std::string& msg, const int max_attempts = MAX_ATTEMPTS);
	bool swrite(const char* msg, const int max_attempts = MAX_ATTEMPTS);
	bool sdisconnect();
};

#endif
