const schema = require('@colyseus/schema');
const Schema = schema.Schema;
const MapSchema = schema.MapSchema;

const Player = require('./Player');

class MyRoomState extends Schema {
    constructor() {
        super();
        this.players = new MapSchema();
    }
}

schema.defineTypes(MyRoomState, {
    players: { map: Player }
});

module.exports = MyRoomState;
