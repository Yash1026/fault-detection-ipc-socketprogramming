import { useEffect, useState } from "react";

import AlertCard from "./components/AlertCard";



export default function App() {
  const [alerts, setAlerts] = useState([]);

  useEffect(() => {
    const ws = new WebSocket("ws://localhost:8080");

    ws.onmessage = (event) => {
      try {
        const parsed = JSON.parse(event.data);
        console.log(parsed);
        setAlerts((prev) => [parsed, ...prev].slice(0, 50));
      } catch {
        console.warn("Invalid alert format:", event.data);
      }
    };

    ws.onopen = () => console.log("[Web] Connected to proxy");
    ws.onerror = (err) => console.error("[Web] WebSocket error:", err);
    ws.onclose = () => console.log("[Web] Connection closed");

    return () => ws.close();
  }, []);

  return (
    <div className="min-h-screen bg-gradient-to-br from-gray-50 to-gray-200">
      {/* Header Section */}
      <header className="bg-white shadow-md sticky top-0 z-10">
        <div className="max-w-6xl mx-auto px-6 py-4 flex justify-between items-center">
          <div>
            <h1 className="text-2xl md:text-3xl font-bold text-gray-800 tracking-tight">
              🏭 Factory Fault Monitoring Dashboard
            </h1>
            <p className="text-gray-500 text-sm md:text-base">
              Real-time alerts from factory equipment via supervisor and network layer
            </p>
          </div>
          <div className="flex items-center gap-2">
            <span className="text-gray-500 text-sm">Active Alerts</span>
            <span className="bg-blue-600 text-white px-3 py-1 rounded-full font-semibold text-sm">
              {alerts.length}
            </span>
          </div>
        </div>
      </header>

      {/* Alerts Grid */}
      <main className="max-w-6xl mx-auto p-6">
        {alerts.length === 0 ? (
          <div className="text-center py-20 text-gray-500">
            <p className="text-xl font-medium">No alerts yet 🔍</p>
            <p className="text-sm mt-2">
              Waiting for supervisor or sensor activity...
            </p>
          </div>
        ) : (
          <div className="grid gap-6 md:grid-cols-2 lg:grid-cols-3">
            {alerts.map((alert, i) => (
              <AlertCard key={i} alert={alert} />
            ))}
          </div>
        )}
      </main>

      {/* Footer */}
      <footer className="bg-white border-t text-center py-4 text-gray-400 text-sm">
        © {new Date().getFullYear()} Factory Monitoring System · Powered by Linux IPC + Socket Programming
      </footer>
    </div>
  );
}
