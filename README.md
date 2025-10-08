# Hybrid Factory Equipment Fault Detection System

This project, developed for a Network Programming course at BITS Pilani, is a fault tolerance system that simulates the monitoring of factory machinery. It uses fundamental OS concepts like Inter-Process Communication (IPC) and signals, combined with TCP socket programming to create a real-time, multi-process monitoring and alert system.

The core goal is to build a realistic system that monitors multiple machines, detects faults based on predefined thresholds (e.g., overheating), and immediately notifies engineers to prevent downtime and safety risks.

## 📋 System Architecture

The system is composed of four main components that work together to collect data, detect faults, and distribute alerts.

![System Architecture Diagram](https://i.imgur.com/your-architecture-diagram.png)
*(Note: You can replace the link above with a screenshot of your architecture diagram from the presentation)*

1.  **Machine (`machine.c`)**: Each machine in the factory is simulated as a separate OS process. It continuously generates sensor readings (like temperature and load) and sends them to the Supervisor via a System V Message Queue.

2.  **Supervisor (`supervisor.c`)**: This is the central controller on the factory floor. It aggregates readings from all machine processes, compares them against thresholds defined in `faultsys.conf`, and raises an alert if a fault is detected.

3.  **Alert Server (`alert_server.c`)**: A central daemon process that runs in the background. When the Supervisor detects a fault, it sends an alert message over a TCP socket to this server. The server logs the alert and broadcasts it to all connected Engineer Clients.

4.  **Engineer Client (`client.c`)**: This program represents the dashboard for an engineer or operator. It connects to the Alert Server and listens for incoming fault alerts, displaying them in real-time on the console.

## ✨ Key Features

* **Real-time Monitoring**: The supervisor continuously captures machine metrics like temperature and load with updates every few seconds.
* **Threshold-Based Fault Detection**: The system triggers an alert within a second if a machine metric exceeds a configurable threshold.
* **IPC for Data Aggregation**: Uses System V Message Queues for efficient communication between the simulated machine processes and the central supervisor.
* **Client-Server Networking**: Employs a robust TCP client-server model to send alerts from the factory floor (Supervisor) to a central server, which then broadcasts them to multiple engineer clients.
* **Daemon Process**: The alert server runs as a background daemon, ensuring continuous and reliable operation without user intervention.
* **Configuration File**: Thresholds for different machines and metrics can be easily managed through the `faultsys.conf` file.

## 🚀 Getting Started

Follow these instructions to get the project compiled and running on a Linux system.

### Prerequisites

You need a Linux environment (like Ubuntu) with the GCC compiler installed.

### Compilation

Compile the four main components using the following commands:

```bash
# Compile the machine simulator
gcc machine.c -o machine

# Compile the supervisor
gcc supervisor.c -o supervisor -lpthread

# Compile the alert server
gcc alert_server.c -o alert_server -lpthread

# Compile the engineer client
gcc client.c -o client
