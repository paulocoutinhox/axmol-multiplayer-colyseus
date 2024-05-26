#pragma once

#include "axmol.h"

#include "Colyseus/Client.hpp"
#include "Colyseus/Room.hpp"
#include "ColyseusData/RoomState.hpp"

class MainScene : public ax::Scene {
    enum class GameState {
        init = 0,
        update,
    };

public:
    bool init() override;
    void update(float delta) override;

private:
    GameState _gameState = GameState::init;

    Client *client;
    Room<RoomState> *room;
    ax::Sprite *playerSprite;
    std::map<std::string, ax::Sprite *> playerSprites;

    bool moveUp = false;
    bool moveDown = false;
    bool moveLeft = false;
    bool moveRight = false;

    void onKeyPressed(ax::EventKeyboard::KeyCode keyCode, ax::Event *event);
    void onKeyReleased(ax::EventKeyboard::KeyCode keyCode, ax::Event *event);
};
