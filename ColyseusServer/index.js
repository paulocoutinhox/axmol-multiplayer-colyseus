const http = require('http');
const express = require('express');
const colyseus = require('colyseus');
const MyRoom = require('./rooms/MyRoom');

const app = express();
const server = http.createServer(app);
const gameServer = new colyseus.Server({ server });

gameServer.define('my_room', MyRoom);

gameServer.listen(3000, () => {
    console.log('Server is listening on port 3000');
});
