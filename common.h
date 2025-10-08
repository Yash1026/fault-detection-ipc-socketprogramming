#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define MSGQ_IPC_FILE "/tmp/faultsys_ipc"
#define MSGQ_PROJ_ID 'F'
#define MSGQ_KEY_PATH "/tmp"  // fallback

#define SERVER_PORT 9000
#define SERVER_BACKLOG 8
#define LOG_FILE "./faultsys.log"

#define MACHINE_ID_LEN 32
#define METRIC_LEN 16
#define ALERT_MSG_LEN 256

// System V message queue message
struct ipc_msg {
    long mtype; // must be >0
    char machine_id[MACHINE_ID_LEN];
    char metric[METRIC_LEN]; // "temp", "rpm", "load"
    double value;
    unsigned long epoch_ms;
};

// Alert structure (sent as newline-delimited JSON-like string over TCP)
struct alert {
    char machine_id[MACHINE_ID_LEN];
    char metric[METRIC_LEN];
    double value;
    double threshold;
    char severity[16]; // "WARN"/"CRIT"
    unsigned long epoch_ms;
    char msg[ALERT_MSG_LEN];
};

#endif // COMMON_H
