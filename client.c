// client.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"


int main(int argc, char **argv) {
    const char *srv = "127.0.0.1";
    int port = SERVER_PORT;
    if (argc >= 2) srv = argv[1];
    if (argc >= 3) port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, srv, &serv.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv, sizeof serv) < 0) {
        perror("connect");
        exit(1);
    }

    printf("[Client] Connected to %s:%d. Waiting for alerts...\n", srv, port);
    char buf[4096];
    while (1) {
        ssize_t r = recv(sock, buf, sizeof buf -1, 0);
        if (r <= 0) {
            fprintf(stderr, "[Client] Disconnected from server.\n");
            break;
        }
        buf[r] = '\0';
        printf("\n>>> ALERT RECEIVED: %s\n", buf);
    }
    close(sock);
    return 0;
}
