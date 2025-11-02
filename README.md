# Hybrid Factory Equipment Fault Detection System

This project, developed for a Network Programming course at BITS Pilani, is a fault tolerance system that simulates the monitoring of factory machinery. It uses fundamental OS concepts like Inter-Process Communication (IPC) and signals, combined with TCP socket programming to create a real-time, multi-process monitoring and alert system.

The core goal is to build a realistic system that monitors multiple machines, detects faults based on predefined thresholds (e.g., overheating), and immediately notifies engineers to prevent downtime and safety risks.

## 📋 System Architecture

The system is composed of multiple components that work together to collect data, detect faults, and distribute alerts through both console-based and web-based interfaces.

<img width="608" height="376" alt="Screenshot 2025-10-08 at 9 09 05 PM" src="https://github.com/user-attachments/assets/a6ab2707-3dc6-4653-9c6d-4702d9dc6957" />

### Core Components

1.  **Machine (`machine.c`)**: Each machine in the factory is simulated as a separate OS process. It continuously generates sensor readings (like temperature and load) and sends them to the Supervisor via a System V Message Queue.

2.  **Supervisor (`supervisor.c`)**: This is the central controller on the factory floor. It aggregates readings from all machine processes, compares them against thresholds defined in `faultsys.conf`, and raises an alert if a fault is detected.

3.  **Alert Server (`alert_server.c`)**: A multi-threaded TCP server that runs in the background. It listens on two ports:
    - Port 9000: Accepts connections from Engineer Clients (console clients and proxy)
    - Port 9001: Accepts alert messages from the Supervisor
    When the Supervisor detects a fault, it sends an alert message over TCP to port 9001. The server logs the alert and broadcasts it to all connected clients (both console and web clients via the proxy).

4.  **Engineer Client (`client.c`)**: A console-based TCP client that connects directly to the Alert Server (port 9000) and displays incoming fault alerts in real-time on the terminal.

### Web Interface Components

5.  **Proxy Server (`proxy/server.js`)**: A Node.js WebSocket proxy server that bridges the TCP-based Alert Server with web clients. It:
    - Connects to the Alert Server on port 9000 (same port as console clients)
    - Exposes a WebSocket server on port 8080
    - Forwards alert messages from the TCP server to all connected web clients via WebSocket
    - Enables real-time communication between the C-based alert system and modern web browsers

6.  **Web Client (`web-client/`)**: A React-based web application that provides a modern, responsive dashboard for monitoring factory equipment. It:
    - Connects to the Proxy Server via WebSocket (ws://localhost:8080)
    - Displays alerts in real-time with a beautiful, card-based UI
    - Shows machine information, metric values, thresholds, severity levels, and timestamps
    - Maintains a history of the last 50 alerts
    - Uses Tailwind CSS for styling

## ✨ Key Features

* **Real-time Monitoring**: The supervisor continuously captures machine metrics like temperature and load with updates every few seconds.
* **Threshold-Based Fault Detection**: The system triggers an alert within a second if a machine metric exceeds a configurable threshold.
* **IPC for Data Aggregation**: Uses System V Message Queues for efficient communication between the simulated machine processes and the central supervisor.
* **Client-Server Networking**: Employs a robust TCP client-server model to send alerts from the factory floor (Supervisor) to a central server, which then broadcasts them to multiple engineer clients.
* **Multi-Interface Support**: Alerts can be viewed through both console-based clients and a modern web dashboard, providing flexibility for different user preferences.
* **WebSocket Integration**: A Node.js proxy server bridges TCP-based alert system with modern web browsers, enabling real-time updates in the browser without page refreshes.
* **Modern Web Dashboard**: React-based web interface with Tailwind CSS styling, displaying alerts in an intuitive card-based layout with color-coded severity levels.
* **Daemon Process**: The alert server runs as a background daemon, ensuring continuous and reliable operation without user intervention.
* **Configuration File**: Thresholds for different machines and metrics can be easily managed through the `faultsys.conf` file.

## 🚀 Getting Started

Follow these instructions to get the project compiled and running on a Linux system.

### Prerequisites

You need a Linux environment (like Ubuntu) with the following installed:

1. **GCC compiler** (for compiling C programs):
   ```bash
   sudo apt-get update
   sudo apt-get install build-essential
   ```

2. **Node.js and npm** (for running the proxy and web client):
   ```bash
   # Install Node.js and npm (version 14 or higher recommended)
   curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
   sudo apt-get install -y nodejs
   # Verify installation
   node --version
   npm --version
   ```

### Compilation

Compile the C components using the following command:

```bash
# Compile all the simulators (machine, alert server, client and supervisor)
make
```

### Installing Node.js Dependencies

Install dependencies for both the proxy server and web client:

```bash
# Install proxy dependencies
cd proxy
npm install
cd ..

# Install web client dependencies
cd web-client
npm install
cd ..
```

### Running the System

Open multiple terminal windows and run the components in the following order:

#### Terminal 1: Alert Server
Start the Alert Server, which listens on ports 9000 (clients) and 9001 (supervisor alerts):
```bash
./alert_server
```
You should see:
```
[Server] Listening for clients on port 9000
[Server] Listening for supervisor alerts on port 9001
```

#### Terminal 2: Proxy Server
Start the WebSocket proxy that bridges the TCP server to web clients:
```bash
cd proxy
npm start
```
You should see:
```
[Proxy] WebSocket listening on ws://localhost:8080
```

#### Terminal 3: Web Client (React App)
Start the React development server for the web dashboard:
```bash
cd web-client
npm start
```
This will automatically open your browser at `http://localhost:3000` showing the web dashboard.

#### Terminal 4: Console Client (Optional)
Alternatively or in addition, you can run the console-based client:
```bash
./client
```

#### Terminal 5: Supervisor
Start the Supervisor, which monitors machines and sends alerts:
```bash
./supervisor
```

#### Terminal 6+: Machine Processes
Start one or more machine simulators. Each machine needs a unique ID, the metric it reports, a value range, and an update interval:
```bash
# This machine will operate normally
./machine Machine-1 temp 20 60 2000

# This machine is configured to trip the 80-degree threshold
./machine Machine-2 temp 75 85 1500

# You can start multiple machines with different configurations
./machine Machine-3 load 10 40 3000
```

### Expected Behavior

Once all components are running:
- When a machine exceeds its threshold (e.g., `Machine-2` with temperature 75-85 against an 80-degree threshold), the Supervisor detects it
- The Supervisor sends an alert to the Alert Server on port 9001
- The Alert Server broadcasts the alert to:
  - Console clients (if running `./client`) - you'll see alerts printed in the terminal
  - Web clients via the Proxy Server - you'll see alert cards appear in the browser dashboard at `http://localhost:3000`
- Alerts are displayed in real-time with machine ID, metric, value, threshold, severity, and timestamp

---
## 📂 Codebase Structure

### C Components (Core System)
* `machine.c`: Simulates a factory machine, generates random metric data, and sends it to an IPC message queue.
* `supervisor.c`: Reads data from the message queue, checks for threshold violations based on `faultsys.conf`, and sends alerts to the server.
* `alert_server.c`: A multi-threaded TCP server that listens on two ports:
  - Port 9000: Accepts console client connections
  - Port 9001: Receives alert messages from supervisors
  - Broadcasts alerts to all connected clients
* `client.c`: A simple TCP client that connects to the server (port 9000) and prints any received alert messages to the console.
* `common.h`: A header file containing shared data structures (like `ipc_msg` and `alert`), constants, and includes for all components.
* `faultsys.conf`: The configuration file for defining fault thresholds for different machines and metrics.
* `Makefile`: Build configuration for compiling all C components.

### Web Interface Components
* `proxy/`: Node.js WebSocket proxy server
  - `server.js`: Main proxy server that bridges TCP (port 9000) to WebSocket (port 8080)
  - `package.json`: Node.js dependencies (requires `ws` package)
* `web-client/`: React-based web dashboard
  - `src/App.js`: Main React component that connects to WebSocket and manages alert state
  - `src/components/AlertCard.jsx`: Component for displaying individual alert cards with styling
  - `package.json`: React dependencies including Tailwind CSS for styling
  - Uses WebSocket API to receive real-time alerts from the proxy

---

## 👥 Author

* **Rawal Yash**
