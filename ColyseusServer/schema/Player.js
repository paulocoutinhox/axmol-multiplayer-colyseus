const schema = require('@colyseus/schema');
const Schema = schema.Schema;

class Player extends Schema {
    constructor() {
        super();
        this.x = 0;
        this.y = 0;
    }
}

schema.defineTypes(Player, {
    x: "number",
    y: "number"
});

module.exports = Player;
