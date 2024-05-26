const http = require('http');
const express = require('express');
const colyseus = require('colyseus');
const WebSocketTransport = require('@colyseus/ws-transport').WebSocketTransport;
const MyRoom = require('./rooms/MyRoom');

const app = express();
const server = http.createServer(app);

const transport = new WebSocketTransport({
    pingInterval: 3000,
    pingMaxRetries: 3,
    server: server,
    verifyClient: (info, done) => {
        done(true);
    }
});

const gameServer = new colyseus.Server({
    transport: transport
});

gameServer.define('my_room', MyRoom);

gameServer.listen(3000, () => {
    console.log('Server is listening on port 3000');
});
