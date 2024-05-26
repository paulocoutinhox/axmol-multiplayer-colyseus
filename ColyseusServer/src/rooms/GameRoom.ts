import { Client, Room } from "colyseus";

import { Player } from "../entities/Player";
import { StateHandler } from "./StateHandler";

export class GameRoom extends Room<StateHandler> {
    maxClients = 8;

    onCreate(options) {
        this.setSimulationInterval(() => this.onUpdate());
        this.setState(new StateHandler());

        // example
        /*
        this.onMessage("xyz", (client, message) => {
            this.state.players.get(client.sessionId).abc = "xyz";
        });
        */

        this.onMessage("move", (client, message) => {
            const player = this.state.players.get(client.sessionId);

            if (player) {
                player.position.x = message.x;
                player.position.y = message.y;

                this.state.players.set(client.sessionId, player);
            }
        });
    }

    onJoin(client) {
        const player = new Player();
        player.name = `Player ${this.clients.length}`;
        player.position.x = Math.random();
        player.position.y = Math.random();

        this.state.players.set(client.sessionId, player);
    }

    onUpdate() {
        // example
        /*
        this.state.players.forEach((player, sessionId) => {
            player.position.x += player.position.x * 0.1;
        });
        */
    }

    onLeave(client: Client) {
        this.state.players.delete(client.sessionId);
    }

    onDispose() {
        // ignore
    }

}
