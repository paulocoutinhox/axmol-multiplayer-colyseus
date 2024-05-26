#include <iostream>
#include <sstream>

#include "Connection.hpp"
#include "Protocol.hpp"
#include "msgpack.hpp"

using namespace ax;
using namespace ax::network;

Connection::Connection(const std::string &_endpoint) {
    endpoint = _endpoint;
}

Connection::~Connection() {
    AX_SAFE_DELETE(_ws);
}

void Connection::open() {
    _ws = new WebSocket();

    if (!_ws->open(this, endpoint)) {
        if (_onError) {
            _onError(0, "Failed to initialize WebSocket");
        }

        this->onClose(_ws);
    }
}

WebSocket::State Connection::getReadyState() {
    return _ws->getReadyState();
}

void Connection::close() {
    _ws->closeAsync();
}

void Connection::onOpen(WebSocket *ws) {
    if (_onOpen) {
        _onOpen();
    }
}

void Connection::onMessage(WebSocket *ws, const WebSocket::Data &data) {
    if (_onMessage) {
        _onMessage(data);
    }
}

void Connection::onClose(WebSocket *ws) {
    AX_SAFE_DELETE(ws);

    if (_onClose) {
        _onClose();
    }
}

void Connection::onError(WebSocket *ws, const WebSocket::ErrorCode &error) {
    if (_onError) {
        std::string message = "";

        switch (error) {
        case WebSocket::ErrorCode::CONNECTION_FAILURE:
            message = "CONNECTION_FAILURE";
            break;
        case WebSocket::ErrorCode::TIME_OUT:
            message = "TIME_OUT";
            break;
        default:
            message = "UNKNOWN";
            break;
        }

        _onError(0, message);
    }
}
