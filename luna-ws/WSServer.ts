import { WebSocketServer } from "ws";

const wss = new WebSocketServer({ port: 3001, path: "/" });

wss.on("connection", (ws, req) => {
  console.log(
    `New Client connected ${req.socket.remoteAddress}`,
    "Total clients:",
    wss.clients.size
  );

  ws.on("message", (data) => {
    // Broadcast to all clients
    wss.clients.forEach((client) => {
      if (client.readyState === 1) {
        client.send(data.toString());
      }
    });
  });

  ws.on("close", () => {
    console.log(
      `${req.socket.remoteAddress} Client disconnected`,
      "Total clients:",
      wss.clients.size
    );
  });
});

console.log("WebSocket server is running on ws://localhost:3001");
