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
    struct timeval selectTimeOut = {.tv_usec = 100000};
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
