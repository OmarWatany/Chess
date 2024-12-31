#ifndef NETWORKING_H
#define NETWORKING_H

#if defined(_WIN32)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#if defined(_WIN32)
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())
#else
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#endif

#include "chess.h"
#define PORT "6969"

SOCKET serverSocket(const char *address, const char *port);
SOCKET connectHost(const char *address, const char *port);
void nonblock(SOCKET fd);
int sendMessage(SOCKET sock, Message *msg);
Message getMessage(SOCKET sock);
int enableKeepalive(SOCKET sock);

extern fd_set default_set;

#endif // !NETWORKING_H
