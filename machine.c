// machine.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "common.h"

unsigned long now_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000UL + ts.tv_nsec / 1000000UL;
}

double rand_range(double min, double max) {
    return min + (double)rand() / RAND_MAX * (max - min);
}

int main(int argc, char **argv) {
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <machine_id> <metric> <min> <max> <interval_ms>\n", argv[0]);
        fprintf(stderr, "Example: %s Machine-1 temp 20 90 2000\n", argv[0]);
        exit(1);
    }
    char *machine_id = argv[1];
    char *metric = argv[2];
    double min = atof(argv[3]);
    double max = atof(argv[4]);
    int interval = atoi(argv[5]);

    srand(time(NULL) ^ getpid());

    // create/get msg queue
    key_t key = ftok(MSGQ_KEY_PATH, MSGQ_PROJ_ID);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }
    int msqid = msgget(key, 0666 | IPC_CREAT);
    if (msqid == -1) {
        perror("msgget");
        exit(1);
    }

    struct ipc_msg msg;
    msg.mtype = 1; // single type for all machines to supervisor

    strncpy(msg.machine_id, machine_id, MACHINE_ID_LEN-1);
    msg.machine_id[MACHINE_ID_LEN-1] = '\0';
    strncpy(msg.metric, metric, METRIC_LEN-1);
    msg.metric[METRIC_LEN-1] = '\0';

    printf("[Machine %s] started (metric=%s interval=%d ms)\n", machine_id, metric, interval);

    while (1) {
        double value = rand_range(min, max);
        msg.value = value;
        msg.epoch_ms = now_ms();

        // send
        if (msgsnd(msqid, &msg, sizeof(struct ipc_msg) - sizeof(long), 0) == -1) {
            perror("msgsnd");
        }
        // print local view
        printf("[Machine %s] %s=%.2f\n", machine_id, metric, value);

        usleep(interval * 1000);
    }
    return 0;
}
