# Hybrid Factory Equipment Fault Detection System

This project, developed for a Network Programming course at BITS Pilani, is a fault tolerance system that simulates the monitoring of factory machinery. It uses fundamental OS concepts like Inter-Process Communication (IPC) and signals, combined with TCP socket programming to create a real-time, multi-process monitoring and alert system.

The core goal is to build a realistic system that monitors multiple machines, detects faults based on predefined thresholds (e.g., overheating), and immediately notifies engineers to prevent downtime and safety risks.

## 📋 System Architecture

The system is composed of four main components that work together to collect data, detect faults, and distribute alerts.

<img width="608" height="376" alt="Screenshot 2025-10-08 at 9 09 05 PM" src="https://github.com/user-attachments/assets/a6ab2707-3dc6-4653-9c6d-4702d9dc6957" />

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
# Compile all the simulators(machine, alert server, client and supervisor)
make
```

### Running the System

Open multiple terminal windows and run the components in the following order, as outlined in the project's demo script:

1.  **Start the Alert Server** 📡: This process will run in the background, waiting for connections from the supervisor and clients.
    ```bash
    ./alert_server
    ```
    
2.  **Start the Engineer Client** 🧑‍💻: This client will connect to the server and display any alerts that are broadcast.
    ```bash
    ./client
    ```

3.  **Start the Supervisor** 🕵️: This will begin listening for data from the machine processes.
    ```bash
    ./supervisor
    ```

4.  **Start one or more Machines** ⚙️: Each machine needs a unique ID, the metric it reports, a value range, and an update interval. To trigger a fault, set the min/max range to cross the threshold defined in `faultsys.conf` (e.g., 75-85 to trip an 80-degree threshold).
    ```bash
    # This machine will operate normally
    ./machine Machine-1 temp 20 60 2000

    # This machine is configured to trip the 80-degree threshold
    ./machine Machine-2 temp 75 85 1500
    ```

You will now see the client receive a real-time alert whenever `Machine-2` generates a temperature reading that exceeds its configured threshold.

---
## 📂 Codebase Structure

* `machine.c`: Simulates a factory machine, generates random metric data, and sends it to an IPC message queue.
* `supervisor.c`: Reads data from the message queue, checks for threshold violations based on `faultsys.conf`, and sends alerts to the server.
* `alert_server.c`: A multi-threaded TCP server that listens for alerts from the supervisor and broadcasts them to all connected clients.
* `client.c`: A simple TCP client that connects to the server and prints any received alert messages.
* `common.h`: A header file containing shared data structures (like `ipc_msg` and `alert`), constants, and includes for all components.
* `faultsys.conf`: The configuration file for defining fault thresholds for different machines and metrics.

---

## 👥 Author

* **Rawal Yash**
