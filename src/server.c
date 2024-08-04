#include <assert.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOCKET int

SOCKET socketCreate() {
    struct addrinfo *addr = NULL, hints = {
                                      .ai_family = AF_INET,
                                      .ai_socktype = SOCK_STREAM,
                                      .ai_flags = AI_PASSIVE,
                                  };
    int rc = getaddrinfo(0, "6969", &hints, &addr);
    assert(rc == 0);
    SOCKET sock = socket(addr->ai_family, addr->ai_socktype, 0);
    rc = bind(sock, addr->ai_addr, addr->ai_addrlen);
    assert(rc == 0);
    freeaddrinfo(addr);
    return sock;
}

int main() {
    SOCKET sock = socketCreate();

    close(sock);
    return 0;
}
