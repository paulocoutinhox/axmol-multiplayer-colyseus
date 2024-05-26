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

    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    label->setPosition(Vec2(origin.x + visibleSize.width / 2, origin.y + visibleSize.height - label->getContentSize().height));
    this->addChild(label);

    client = new Client("ws://localhost:3000");
    client->joinOrCreate<RoomStateSchema>("my_room", nullptr, [&](Room *room) {
        this->room = room;
        this->room->onStateChange = CC_CALLBACK_1(GameScene::onStateChange, this);
        this->room->onMessage = CC_CALLBACK_2(GameScene::onMessage, this);

        // initialize player sprite
        playerSprite = Sprite::create("player.png");
        playerSprite->setPosition(Vec2(240, 160));
        this->addChild(playerSprite);

        // add keyboard event listener
        auto eventListener = EventListenerKeyboard::create();
        eventListener->onKeyPressed = CC_CALLBACK_2(GameScene::onKeyPressed, this);
        eventListener->onKeyReleased = CC_CALLBACK_2(GameScene::onKeyReleased, this);
        this->_eventDispatcher->addEventListenerWithSceneGraphPriority(eventListener, playerSprite);

        scheduleUpdate();
    });

    scheduleUpdate();

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

void MainScene::onStateChange(State *state) {
    // update players based on the state
    for (auto &kv : state->players) {
        const std::string &playerId = kv.first;
        auto &playerData = kv.second;

        if (playerSprites.find(playerId) == playerSprites.end()) {
            auto sprite = Sprite::create("player.png");
            this->addChild(sprite);
            playerSprites[playerId] = sprite;
        }

        playerSprites[playerId]->setPosition(Vec2(playerData["x"].GetFloat(), playerData["y"].GetFloat()));
    }

    // remove disconnected players
    std::vector<std::string> toRemove;
    for (auto &kv : playerSprites) {
        if (state->players.find(kv.first) == state->players.end()) {
            this->removeChild(kv.second);
            toRemove.push_back(kv.first);
        }
    }

    for (const auto &playerId : toRemove) {
        playerSprites.erase(playerId);
    }
}

void MainScene::onMessage(const std::string &type, const std::map<std::string, float> &message) {
    // handle other messages from server if needed
}

void MainScene::update(float delta) {
    switch (_gameState) {

    case GameState::init: {
        _gameState = GameState::update;
        break;
    }

    case GameState::update: {
        Vec2 pos = playerSprite->getPosition();

        if (moveUp)
            pos.y += 100 * delta;
        if (moveDown)
            pos.y -= 100 * delta;
        if (moveLeft)
            pos.x -= 100 * delta;
        if (moveRight)
            pos.x += 100 * delta;

        playerSprite->setPosition(pos);

        // send movement data to server
        if (room) {
            room->send("move", {{"x", pos.x}, {"y", pos.y}});
        }

        break;
    }
    }
}
