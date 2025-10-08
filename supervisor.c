// supervisor.c
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "common.h"

#define CONFIG_FILE "./faultsys.conf"
#define SERVER_ADDR "127.0.0.1" // central server local for demo

// Simple threshold structure (only supports one metric: temp)
typedef struct {
    char machine_prefix[32];
    char metric[16];
    double threshold;
} threshold_t;

threshold_t thresholds[16];
int threshold_count = 0;

int msqid = -1;
int server_sock = -1;

volatile sig_atomic_t last_signal = 0;

void sigusr1_handler(int signo) {
    last_signal = signo;
    fprintf(stderr, "[Supervisor] SIGUSR1 received! Local alert handler triggered.\n");
}

unsigned long now_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000UL + ts.tv_nsec / 1000000UL;
}

void load_config() {
    FILE *f = fopen(CONFIG_FILE, "r");
    if (!f) {
        fprintf(stderr, "[Supervisor] No config file found (%s). Using default threshold: temp>80\n", CONFIG_FILE);
        // default
        strcpy(thresholds[0].machine_prefix, "Machine");
        strcpy(thresholds[0].metric, "temp");
        thresholds[0].threshold = 80.0;
        threshold_count = 1;
        return;
    }
    threshold_count = 0;
    char line[256];
    while (fgets(line, sizeof line, f)) {
        if (line[0]=='#' || strlen(line)<3) continue;
        char prefix[32], metric[16];
        double th;
        if (sscanf(line, "%31s %15s %lf", prefix, metric, &th) == 3) {
            strncpy(thresholds[threshold_count].machine_prefix, prefix, 31);
            strncpy(thresholds[threshold_count].metric, metric, 15);
            thresholds[threshold_count].threshold = th;
            threshold_count++;
            if (threshold_count >= 16) break;
        }
    }
    fclose(f);
    fprintf(stderr, "[Supervisor] Loaded %d thresholds from %s\n", threshold_count, CONFIG_FILE);
}

double find_threshold_for(const char *machine_id, const char *metric) {
    for (int i=0;i<threshold_count;i++) {
        if (strcmp(metric, thresholds[i].metric)==0) {
            // prefix match
            if (strncmp(machine_id, thresholds[i].machine_prefix, strlen(thresholds[i].machine_prefix))==0) {
                return thresholds[i].threshold;
            }
        }
    }
    return 1e18; // no threshold (very large)
}

int connect_to_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return -1; }
    struct sockaddr_in serv;
    serv.sin_family = AF_INET;
    serv.sin_port = htons(SERVER_PORT + 1);
    inet_pton(AF_INET, SERVER_ADDR, &serv.sin_addr);
    if (connect(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

int send_alert_to_server(struct alert *a) {
    if (server_sock < 0) {
        server_sock = connect_to_server();
        if (server_sock < 0) {
            fprintf(stderr, "[Supervisor] Cannot connect to server; will retry later.\n");
            return -1;
        }
    }
    // format as JSON-like string (simple)
    char buf[512];
    int n = snprintf(buf, sizeof buf,
        "{\"machine\":\"%s\",\"metric\":\"%s\",\"value\":%.2f,\"threshold\":%.2f,\"severity\":\"%s\",\"ts\":%lu,\"msg\":\"%s\"}\n",
        a->machine_id, a->metric, a->value, a->threshold, a->severity, a->epoch_ms, a->msg);
    int sent = 0;
    while (sent < n) {
        int r = send(server_sock, buf + sent, n - sent, 0);
        if (r < 0) {
            perror("[Supervisor] send");
            close(server_sock);
            server_sock = -1;
            return -1;
        }
        sent += r;
    }
    close(server_sock);
    server_sock = -1;
    return 0;
}

void process_msg(struct ipc_msg *m) {
    printf("[Supervisor] Received: %s %s=%.2f at %lu\n", m->machine_id, m->metric, m->value, m->epoch_ms);
    double thr = find_threshold_for(m->machine_id, m->metric);
    if (thr < 1e17 && m->value > thr) {
        // Fault detected
        fprintf(stderr, "[Supervisor] FAULT: %s %s=%.2f > %.2f\n", m->machine_id, m->metric, m->value, thr);
        // local signal
        raise(SIGUSR1);
        // prepare alert
        struct alert a;
        strncpy(a.machine_id, m->machine_id, MACHINE_ID_LEN-1);
        strncpy(a.metric, m->metric, METRIC_LEN-1);
        a.value = m->value;
        a.threshold = thr;
        a.epoch_ms = now_ms();
        strcpy(a.severity, "CRIT");
        snprintf(a.msg, sizeof a.msg, "%s exceeded threshold", m->metric);
        // send to central server
        if (send_alert_to_server(&a) == 0) {
            printf("[Supervisor] Alert sent to server for %s\n", a.machine_id);
        }
    }
}

void *ipc_reader_thread(void *arg) {
    (void)arg;
    struct ipc_msg m;
    while (1) {
        ssize_t r = msgrcv(msqid, &m, sizeof(struct ipc_msg)-sizeof(long), 0, 0);
        if (r < 0) {
            perror("[Supervisor] msgrcv");
            sleep(1);
            continue;
        }
        process_msg(&m);
    }
    return NULL;
}


int main() {
    signal(SIGUSR1, sigusr1_handler);
    load_config();

    // set up message queue
    key_t key = ftok(MSGQ_KEY_PATH, MSGQ_PROJ_ID);
    if (key == -1) { perror("ftok"); exit(1); }
    msqid = msgget(key, 0666 | IPC_CREAT);
    if (msqid == -1) { perror("msgget"); exit(1); }

    // spawn IPC reader thread
    pthread_t ipc_thread;
    if (pthread_create(&ipc_thread, NULL, ipc_reader_thread, NULL) != 0) {
        perror("pthread_create");
        exit(1);
    }

    printf("[Supervisor] Running. Listening for machine readings...\n");

    // main loop can monitor last_signal or perform housekeeping
    while (1) {
        if (last_signal) {
            // already handled in handler; reset flag
            last_signal = 0;
        }
        sleep(1);
    }

    return 0;
}
