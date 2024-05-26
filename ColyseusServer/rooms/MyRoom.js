const colyseus = require('colyseus');
const MyRoomState = require('../schema/MyRoomState');
const Player = require('../schema/Player');

class MyRoom extends colyseus.Room {
    onCreate(options) {
        this.setState(new MyRoomState());

        this.onMessage('move', (client, message) => {
            const player = this.state.players.get(client.sessionId);
            if (player) {
                player.x = message.x;
                player.y = message.y;
            }
        });
    }

    onJoin(client, options) {
        this.state.players.set(client.sessionId, new Player());
    }

    onLeave(client, consented) {
        this.state.players.delete(client.sessionId);
    }
}

module.exports = MyRoom;
