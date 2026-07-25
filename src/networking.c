#include "networking.h"
#include "chess.h"
#include "gringbuffer.h"
#include <alloca.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

fd_set default_set;

int sendMessage(SOCKET sock, Message *msg) {
    fd_set writeMask = default_set;
    const int MsgSize = sizeof(*msg);
    int bytes = 0, sum = 0;
    struct timeval selectTimeOut = {.tv_usec = 100000};
    select(sock + 1, 0, &writeMask, 0, &selectTimeOut);
    if (!FD_ISSET(sock, &writeMask)) return 0;
    do {
        bytes = send(sock, msg, MsgSize, 0);
        sum += bytes;
    } while (sum < MsgSize && bytes < MsgSize);
    return 0;
}

Message getMessage(SOCKET sock) {
    Message msg = {0};
    if (sock < 0) return msg;
    fd_set readMask = default_set;
    const int MsgSize = sizeof(Message);
    int sum = 0, bytes = 0;
    struct timeval selectTimeOut = {
        .tv_sec = 0,
        .tv_usec = 0,
    };
    select(sock + 1, &readMask, 0, 0, &selectTimeOut);
    if (!FD_ISSET(sock, &readMask)) return msg;
    do {
        bytes = recv(sock, ringbuffer_write_idx(&rbuffer), ringbuffer_remaining_sapce(&rbuffer), 0);
        if (bytes == 0) return (Message){.kind = PEER_CLOSED};
        sum += bytes;
        ringbuffer_commit_write(&rbuffer, bytes);
    } while (bytes < MsgSize && sum < MsgSize);
    memcpy(&msg, ring_read_return(&rbuffer, MsgSize), MsgSize);
    return msg;
}

void nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        fprintf(stderr, "Invalid flags on nonblock. %m\n");
        exit(1);
    }

    int rc = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (rc) {
        fprintf(stderr, "Can't set nonblocking. %m\n");
        exit(1);
    }
}

SOCKET serverSocket(const char *address, const char *port) {
    struct addrinfo *addr = NULL, hints = {
                                      .ai_family = AF_INET,
                                      .ai_socktype = SOCK_STREAM,
                                      .ai_flags = AI_PASSIVE,
                                  };
    assert(getaddrinfo(address, port, &hints, &addr) == 0);
    SOCKET servSock = socket(addr->ai_family, addr->ai_socktype, 0);
    int rc = bind(servSock, addr->ai_addr, addr->ai_addrlen);
    assert(rc == 0);
    nonblock(servSock);
    freeaddrinfo(addr);
    return servSock;
}

SOCKET connectHost(const char *address, const char *port) {
    struct addrinfo *addr = NULL, hints = {
                                      .ai_family = AF_INET,
                                      .ai_socktype = SOCK_STREAM,
                                  };
    SOCKET servSock = -1;
    assert(getaddrinfo(address, port, &hints, &addr) == 0);
    servSock = socket(addr->ai_family, addr->ai_socktype, 0);
    assert(servSock > 0);
    if (connect(servSock, addr->ai_addr, addr->ai_addrlen)) {
        printf("couldn't connect becaue of %m\n");
        exit(1);
    }
    nonblock(servSock);
    freeaddrinfo(addr);
    return servSock;
}

SOCKET startServer(const char *address, const char *port) {
    SOCKET hostSock = serverSocket(address ? address : "localhost", port ? port : PORT);
    if (!ISVALIDSOCKET(hostSock)) return -1;
    if (listen(hostSock, 2) < 0) {
        CLOSESOCKET(hostSock);
        return -1;
    }
    return hostSock;
}

SOCKET acceptHostConnection(SOCKET hostSock, bool (*on_tick)(void *user_data), void *user_data) {
    if (!ISVALIDSOCKET(hostSock)) return -1;

    struct sockaddr_storage clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    SOCKET peer = -1;

    while (!ISVALIDSOCKET(peer)) {
        if (on_tick && !on_tick(user_data)) {
            return -1;
        }

        peer = accept(hostSock, (struct sockaddr *)&clientAddress, &clientLen);
        if (!ISVALIDSOCKET(peer)) continue;
    }

    nonblock(peer);

    char addressBuffer[100];
    char portNumber[8];
    getnameinfo((struct sockaddr *)&clientAddress, clientLen, addressBuffer, sizeof(addressBuffer), portNumber,
                sizeof(portNumber), NI_NUMERICHOST);
    printf("Client %s:%s connected\n", addressBuffer, portNumber);

    FD_ZERO(&default_set);
    FD_SET(peer, &default_set);
    return peer;
}

SOCKET initClient(const char *address, const char *port) {
    SOCKET peer = connectHost(address ? address : "localhost", port ? port : PORT);
    if (ISVALIDSOCKET(peer)) {
        FD_ZERO(&default_set);
        FD_SET(peer, &default_set);
    }
    return peer;
}

void closeSocket(SOCKET *sock) {
    if (sock && ISVALIDSOCKET(*sock)) {
        CLOSESOCKET(*sock);
        *sock = -1;
    }
}