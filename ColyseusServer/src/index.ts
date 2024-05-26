import { monitor } from "@colyseus/monitor";
import { playground } from "@colyseus/playground";
import { WebSocketTransport } from "@colyseus/ws-transport";
import { Server } from "colyseus";
import cors from "cors";
import express from "express";
import http from "http";
import path from "path";
import { GameRoom } from "./rooms/GameRoom";

export const port = process.env.PORT || 3000;

const app = express();

app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, "public")));

app.use("/playground", playground);
app.use("/monitor", monitor());

// Create HTTP & WebSocket servers
const server = http.createServer(app);
const gameServer = new Server({
    transport: new WebSocketTransport({
        server: server,
        pingInterval: 10000,
        pingMaxRetries: 3,
        verifyClient: (info, done) => {
            done(true);
        }
    })
});

gameServer.define("game", GameRoom);

server.listen(port);
console.log(`Listening on ${port}`);
