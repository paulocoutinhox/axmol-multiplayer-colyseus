#include "MainScene.h"

USING_NS_AX;

using namespace colyseus;

bool MainScene::init() {
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = _director->getVisibleSize();
    auto origin = _director->getVisibleOrigin();
    auto safeArea = _director->getSafeAreaRect();
    auto safeOrigin = safeArea.origin;

    auto label = Label::createWithTTF("Multiplayer Game", "fonts/Marker Felt.ttf", 24);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - label->getContentSize().height));
    addChild(label);

    // add keyboard event listener
    auto eventListener = EventListenerKeyboard::create();
    eventListener->onKeyPressed = AX_CALLBACK_2(MainScene::onKeyPressed, this);
    eventListener->onKeyReleased = AX_CALLBACK_2(MainScene::onKeyReleased, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, this);

    // add touch event listener
    auto touchListener = EventListenerTouchOneByOne::create();
    touchListener->onTouchBegan = AX_CALLBACK_2(MainScene::onTouchBegan, this);
    touchListener->onTouchEnded = AX_CALLBACK_2(MainScene::onTouchEnded, this);
    _eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

    // schedule update to always run
    scheduleUpdate();

    // connect to the server
    client = new Client("ws://192.168.0.113:3000");

    client->joinOrCreate<RoomStateSchema>("game", {}, [this](MatchMakeError *error, Room<RoomStateSchema> *room) {
        if (error) {
            // handle error
            return;
        }

        this->room = room;
        this->room->onStateChange = [this](RoomStateSchema *state) {
            this->onStateChange(state);
        };

        // uncomment if you need to handle specific messages from the server
        // this->room->onMessage = [this](const std::string &type, const msgpack::object &message) {
        //     this->onMessage(type, message);
        // };

        // initialize player sprite once connected
        playerSprite = Sprite::create("player.png");
        playerSprite->setContentSize(Vec2(100, 100));
        playerSprite->setPosition(Vec2(240, 160));
        this->addChild(playerSprite);
    });

    return true;
}

void MainScene::onKeyPressed(EventKeyboard::KeyCode keyCode, Event *event) {
    switch (keyCode) {
    case EventKeyboard::KeyCode::KEY_W:
        moveUp = true;
        break;
    case EventKeyboard::KeyCode::KEY_S:
        moveDown = true;
        break;
    case EventKeyboard::KeyCode::KEY_A:
        moveLeft = true;
        break;
    case EventKeyboard::KeyCode::KEY_D:
        moveRight = true;
        break;
    default:
        break;
    }
}

void MainScene::onKeyReleased(EventKeyboard::KeyCode keyCode, Event *event) {
    switch (keyCode) {
    case EventKeyboard::KeyCode::KEY_W:
        moveUp = false;
        break;
    case EventKeyboard::KeyCode::KEY_S:
        moveDown = false;
        break;
    case EventKeyboard::KeyCode::KEY_A:
        moveLeft = false;
        break;
    case EventKeyboard::KeyCode::KEY_D:
        moveRight = false;
        break;
    default:
        break;
    }
}

void MainScene::onStateChange(RoomStateSchema *state) {
    // update players based on the state
    for (auto &kv : state->players->items) {
        const std::string &playerId = kv.first;
        PlayerSchema *playerData = kv.second;

        if (playerSprites.find(playerId) == playerSprites.end()) {
            auto sprite = Sprite::create("player.png");
            this->addChild(sprite);
            playerSprites[playerId] = sprite;
        }

        playerSprites[playerId]->setPosition(Vec2(playerData->position->x, playerData->position->y));
    }

    // remove disconnected players
    std::vector<std::string> toRemove;
    for (auto &kv : playerSprites) {
        if (!state->players->has(kv.first)) {
            this->removeChild(kv.second);
            toRemove.push_back(kv.first);
        }
    }

    for (const auto &playerId : toRemove) {
        playerSprites.erase(playerId);
    }
}

void MainScene::onMessage(const std::string &type, const msgpack::object &message) {
    // handle other messages from server if needed
}

void MainScene::update(float delta) {
    switch (_gameState) {
    case GameState::init: {
        _gameState = GameState::update;
        break;
    }

    case GameState::update: {
        if (playerSprite) {
            Vec2 pos = playerSprite->getPosition();

            if (moveUp) {
                pos.y += 100 * delta;
            }

            if (moveDown) {
                pos.y -= 100 * delta;
            }

            if (moveLeft) {
                pos.x -= 100 * delta;
            }

            if (moveRight) {
                pos.x += 100 * delta;
            }

            playerSprite->setPosition(pos);

            // send movement data to server if room is connected
            if (room) {
                //                std::map<std::string, float> movementData = {{"x", pos.x}, {"y", pos.y}};
                //                room->send("move", movementData);
            }
        }
    }
    }
}

bool MainScene::onTouchBegan(Touch *touch, Event *event) {
    // Returning true to indicate that we have handled the touch event
    return true;
}

void MainScene::onTouchEnded(Touch *touch, Event *event) {
    if (playerSprite) {
        Vec2 touchLocation = touch->getLocation();
        playerSprite->setPosition(touchLocation);

        // send movement data to server if room is connected
        if (room) {
            //            std::map<std::string, float> movementData = {{"x", touchLocation.x}, {"y", touchLocation.y}};
            //            room->send("move", movementData);
        }
    }
}
