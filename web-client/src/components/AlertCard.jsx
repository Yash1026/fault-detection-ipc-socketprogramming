export default function AlertCard({ alert }) {
  const isCritical = alert.severity === "CRIT";
  const borderColor = isCritical ? "border-red-500" : "border-yellow-400";
  const bgColor = isCritical ? "bg-red-50" : "bg-yellow-50";
  const badgeColor = isCritical
    ? "bg-red-100 text-red-700"
    : "bg-yellow-100 text-yellow-700";

  return (
    <div
      className={`border-l-4 ${borderColor} ${bgColor} shadow-sm rounded-xl p-5 hover:shadow-lg hover:-translate-y-1 transition-all duration-300`}
    >
      <div className="flex justify-between items-center mb-2">
        <h3 className="font-bold text-lg text-gray-800">
          Machine: {alert.machine || "Unknown Machine"}
        </h3>
        <span
          className={`text-xs font-semibold px-3 py-1 rounded-full ${badgeColor}`}
        >
          Severity: {alert.severity}
        </span>
      </div>

      <div className="text-gray-700 text-sm space-y-1">
        <p>
          <span className="font-medium">Metric: {alert.metric}</span>:{" "}
          <span className="font-bold"> Value: {alert.value}</span> (Threshold:{" "}
          {alert.threshold})
        </p>
        <p className="text-xs text-gray-500">Timestamp: {new Date(alert.ts).toLocaleString()}</p>
        <p className="italic mt-1">Message: {alert.msg}</p>
      </div>
    </div>
  );
}
