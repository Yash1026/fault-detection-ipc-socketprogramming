// proxy/server.js
const net = require("net");
const WebSocket = require("ws");

const TCP_SERVER = "127.0.0.1";
const TCP_PORT = 9000; // your alert_server client port
const WS_PORT = 8080;

const wss = new WebSocket.Server({ port: WS_PORT });
console.log(`[Proxy] WebSocket listening on ws://localhost:${WS_PORT}`);

wss.on("connection", (ws) => {
  console.log("[Proxy] Web client connected");

  // Connect to the alert_server TCP port
  const tcpClient = net.createConnection({ host: TCP_SERVER, port: TCP_PORT }, () => {
    console.log("[Proxy] Connected to alert_server");
  });

  tcpClient.on("data", (data) => {
    const msg = data.toString().trim();
    console.log("[Proxy] Received from server:", msg);
    ws.send(msg);
  });

  tcpClient.on("close", () => {
    console.log("[Proxy] TCP connection closed");
    ws.close();
  });

  tcpClient.on("error", (err) => {
    console.error("[Proxy] TCP error:", err.message);
  });

  ws.on("close", () => {
    console.log("[Proxy] Web client disconnected");
    tcpClient.end();
  });
});
