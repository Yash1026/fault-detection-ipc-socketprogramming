// alert_server.c (fixed with dual-port support)
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "common.h"

#define MAX_CLIENTS 32
#define ALERT_PORT (SERVER_PORT + 1)  // new port for supervisors

static int client_listen_fd = -1;
static int alert_listen_fd = -1;
static int clients[MAX_CLIENTS];
static pthread_mutex_t clients_lock = PTHREAD_MUTEX_INITIALIZER;
static FILE *logf = NULL;

void log_alert(const char *s) {
    if (!logf) {
        logf = fopen(LOG_FILE, "a");
        if (!logf) return;
    }
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    char ts[64];
    strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S", &tm);
    fprintf(logf, "[%s] %s\n", ts, s);
    fflush(logf);
}

void broadcast_to_clients(const char *msg) {
    pthread_mutex_lock(&clients_lock);
    for (int i=0;i<MAX_CLIENTS;i++) {
        if (clients[i] > 0) {
            ssize_t n = send(clients[i], msg, strlen(msg), 0);
            if (n <= 0) {
                close(clients[i]);
                clients[i] = 0;
            }
        }
    }
    pthread_mutex_unlock(&clients_lock);
}

void *client_acceptor(void *arg) {
    (void)arg;
    while (1) {
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int fd = accept(client_listen_fd, (struct sockaddr*)&cli, &len);
        if (fd < 0) {
            perror("accept(client)");
            continue;
        }
        pthread_mutex_lock(&clients_lock);
        int placed = 0;
        for (int i=0;i<MAX_CLIENTS;i++) {
            if (clients[i]==0) { clients[i] = fd; placed=1; break; }
        }
        pthread_mutex_unlock(&clients_lock);
        if (!placed) {
            fprintf(stderr, "[Server] Too many clients\n");
        } else {
            fprintf(stderr, "[Server] Client connected from %s:%d\n",
                    inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));
        }
    }
    return NULL;
}

void *alert_receiver(void *arg) {
    (void)arg;
    while (1) {
        struct sockaddr_in saddr;
        socklen_t slen = sizeof(saddr);
        int fd = accept(alert_listen_fd, (struct sockaddr*)&saddr, &slen);
        if (fd < 0) {
            perror("accept(alert)");
            continue;
        }

        char buf[2048];
        ssize_t r = recv(fd, buf, sizeof buf -1, 0);
        if (r > 0) {
            buf[r] = '\0';
            fprintf(stderr, "[Server] Received ALERT: %s\n", buf);
            log_alert(buf);
            broadcast_to_clients(buf);
        }
        close(fd);
    }
    return NULL;
}

int main(void) {
    // setup client listener
    client_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv = {0};
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(SERVER_PORT);
    bind(client_listen_fd, (struct sockaddr*)&serv, sizeof(serv));
    listen(client_listen_fd, SERVER_BACKLOG);

    // setup alert listener
    alert_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in alr = {0};
    alr.sin_family = AF_INET;
    alr.sin_addr.s_addr = INADDR_ANY;
    alr.sin_port = htons(ALERT_PORT);
    bind(alert_listen_fd, (struct sockaddr*)&alr, sizeof(alr));
    listen(alert_listen_fd, SERVER_BACKLOG);

    fprintf(stderr, "[Server] Listening for clients on port %d\n", SERVER_PORT);
    fprintf(stderr, "[Server] Listening for supervisor alerts on port %d\n", ALERT_PORT);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, client_acceptor, NULL);
    pthread_create(&t2, NULL, alert_receiver, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
