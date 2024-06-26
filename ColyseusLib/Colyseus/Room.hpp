#pragma once

#include "Connection.hpp"
#include "Protocol.hpp"
#include <stdio.h>

#include "Serializer/SchemaSerializer.hpp"
#include "Serializer/Serializer.hpp"
#include "Serializer/schema.h"

/*
class IRoom {
public:
    virtual ~IRoom() = default;

    std::string id;
    std::map<std::string, std::string> options;
    virtual void connect(Connection *connection) = 0;
};
*/

template <typename S>
class Room
// : IRoom
{
public:
    Room(const std::string _name) {
        name = _name;
        serializer = new SchemaSerializer<S>();
    }
    ~Room() {}

    // Methods
    void connect(Connection *connection) {
        this->connection = connection;
        this->connection->_onClose = std::bind(&Room::_onClose, this);
        this->connection->_onError = std::bind(&Room::_onError, this, std::placeholders::_1, std::placeholders::_2);
        this->connection->_onMessage = std::bind(&Room::_onMessage, this, std::placeholders::_1);
        this->connection->open();
    }

    void leave(bool consented = true) {
        if (!this->id.empty()) {
            if (consented) {
                unsigned char bytes[1] = {(int)Protocol::LEAVE_ROOM};
                this->connection->send(bytes, sizeof(bytes));
            } else {
                this->connection->close();
            }
        } else {
            if (onLeave) {
                this->onLeave();
            }
        }
    }

    inline void send(unsigned char type) {
        unsigned char message[2] = {(int)Protocol::ROOM_DATA, type};
        this->connection->send(message, sizeof(message));
    }

    template <typename T>
    inline void send(const int32_t &type, T message) {
        std::stringstream ss;
        msgpack::pack(ss, message);
        std::string encoded = ss.str();
        size_t encodedLength = encoded.length();

        unsigned char *bytesToSend = new unsigned char[encodedLength + 2];
        bytesToSend[0] = (unsigned char)Protocol::ROOM_DATA;
        bytesToSend[1] = type;
        memcpy(bytesToSend + 2, encoded.c_str(), encodedLength);

        this->connection->send(bytesToSend, encodedLength + 2);
        delete[] bytesToSend;
    }

    inline void send(const std::string &type) {
        const char *typeBytes = type.c_str();
        size_t typeLength = strlen(typeBytes);

        unsigned char *bytesToSend = new unsigned char[2 + typeLength];
        bytesToSend[0] = (unsigned char)Protocol::ROOM_DATA;
        bytesToSend[1] = typeLength | 0xa0;
        memcpy(bytesToSend + 2, typeBytes, typeLength);

        this->connection->send(bytesToSend, 2 + typeLength);
        delete[] bytesToSend;
    }

    template <typename T>
    inline void send(const std::string &type, T message) {
        const char *typeBytes = type.c_str();
        size_t typeLength = strlen(typeBytes);

        std::stringstream ss;
        msgpack::pack(ss, message);
        std::string encoded = ss.str();
        size_t encodedLength = encoded.length();

        unsigned char *bytesToSend = new unsigned char[2 + typeLength + encodedLength];
        bytesToSend[0] = (unsigned char)Protocol::ROOM_DATA;
        bytesToSend[1] = typeLength | 0xa0;
        memcpy(bytesToSend + 2, typeBytes, typeLength);
        memcpy(bytesToSend + 2 + typeLength, encoded.c_str(), encodedLength);

        this->connection->send(bytesToSend, 2 + typeLength + encodedLength);
        delete[] bytesToSend;
    }

    inline Room<S> *onMessage(int type, std::function<void(const msgpack::object &)> callback) {
        onMessageHandlers[getMessageHandlerKey(type)] = callback;
        return this;
    }

    inline Room<S> *onMessage(const std::string &type, std::function<void(const msgpack::object &)> callback) {
        onMessageHandlers[getMessageHandlerKey(type)] = callback;
        return this;
    }

    S *getState() {
        return this->serializer->getState();
    }

    // Callbacks
    std::function<void()> onJoin;
    std::function<void()> onLeave;
    std::function<void(const int &, const std::string &)> onError;
    std::function<void(S *)> onStateChange;
    // std::function<void(const msgpack::object &)> onMessage;

    std::map<const std::string, std::function<void(const msgpack::object &)>> onMessageHandlers;

    // Properties
    Connection *connection;

    std::string id;
    std::string name;
    std::string sessionId;
    std::string serializerId;

protected:
    void _onClose() {
        if (this->onLeave) {
            this->onLeave();
        }
    }

    void _onError(const int &code, const std::string &message) {
        if (this->onError) {
            this->onError(code, message);
        }
    }

    void _onMessage(const WebSocket::Data &data) {
        size_t len = data.len;
        unsigned const char *bytes = reinterpret_cast<const unsigned char *>(data.bytes);
        // const char *bytes = data.bytes;

        colyseus::schema::Iterator *it = new colyseus::schema::Iterator();
        it->offset = 0;

#ifdef COLYSEUS_DEBUG
        std::cout << "onMessage bytes =>" << bytes << std::endl;
#endif

        unsigned char code = bytes[it->offset++];

        switch ((Protocol)code) {
        case Protocol::JOIN_ROOM: {
#ifdef COLYSEUS_DEBUG
            std::cout << "Colyseus.Room: join error" << std::endl;
#endif

            serializerId = colyseus::schema::decodeString(bytes, it);

            // TODO: instantiate serializer by id
            if (len > it->offset) {
                serializer->handshake(bytes, it->offset);
            }

            if (this->onJoin) {
                this->onJoin();
            }

            unsigned char message[1] = {(int)Protocol::JOIN_ROOM};
            this->connection->send(message, sizeof(message));
            break;
        }
        case Protocol::JOIN_ERROR: {
#ifdef COLYSEUS_DEBUG
            std::cout << "Colyseus.Room: join error" << std::endl;
#endif
            std::string message = colyseus::schema::decodeString(bytes, it);

            if (this->onError) {
                // TODO:
                this->onError(0, message);
            }
            break;
        }
        case Protocol::LEAVE_ROOM: {
#ifdef COLYSEUS_DEBUG
            std::cout << "Colyseus.Room: LEAVE_ROOM" << std::endl;
#endif
            this->leave();

            break;
        }
        case Protocol::ROOM_DATA: {
#ifdef COLYSEUS_DEBUG
            std::cout << "Colyseus.Room: ROOM_DATA" << std::endl;
#endif
            std::string type;

            if (colyseus::schema::numberCheck(bytes, it)) {
                type = getMessageHandlerKey(colyseus::schema::decodeNumber(bytes, it));
            } else {
                type = getMessageHandlerKey(colyseus::schema::decodeString(bytes, it));
            }

            std::map<const std::string, std::function<void(const msgpack::object &)>>::iterator found;
            found = onMessageHandlers.find(type);

            if (found != onMessageHandlers.end()) {
                if (len > it->offset) {
                    const char *thebytes = data.bytes;
                    msgpack::object_handle oh = msgpack::unpack(thebytes, len, it->offset);
                    msgpack::object data = oh.get();
                    found->second(data);
                } else {
                    msgpack::object empty;
                    found->second(empty);
                }
            } else {
                std::cout << "Room::onMessage() missing for type => " << type << std::endl;
            }

            break;
        }
        case Protocol::ROOM_STATE: {
#ifdef COLYSEUS_DEBUG
            std::cout << "Colyseus.Room: ROOM_STATE" << std::endl;
#endif
            this->setState(bytes, it->offset, len);
            break;
        }
        case Protocol::ROOM_STATE_PATCH: {
#ifdef COLYSEUS_DEBUG
            std::cout << "Colyseus.Room: ROOM_STATE_PATCH" << std::endl;
#endif
            this->applyPatch(bytes, it->offset, len);

            break;
        }
        case Protocol::ROOM_DATA_BYTES: {
#ifdef COLYSEUS_DEBUG
            std::cout << "Colyseus.Room: ROOM_DATA_BYTES" << std::endl;
#endif
            it->offset = 1;

            std::string type;

            if (colyseus::schema::numberCheck(bytes, it)) {
                type = getMessageHandlerKey(colyseus::schema::decodeNumber(bytes, it));
            } else {
                type = getMessageHandlerKey(colyseus::schema::decodeString(bytes, it));
            }

            std::map<const std::string, std::function<void(const msgpack::object &)>>::iterator found;
            found = onMessageHandlers.find(type);

            if (found != onMessageHandlers.end()) {
                if (len > it->offset) {
                    const char *thebytes = data.bytes;
                    msgpack::object_handle oh = msgpack::unpack(thebytes, len, it->offset);
                    msgpack::object data = oh.get();
                    found->second(data);
                } else {
                    msgpack::object empty;
                    found->second(empty);
                }
            } else {
                std::cout << "Room::onMessage() missing for type => " << type << std::endl;
            }

            break;
        }
        default: {
            break;
        }
        }

        delete it;
    }

    void setState(unsigned const char *bytes, int offset, int length) {
        this->serializer->setState(bytes, offset, length);

        if (onStateChange) {
            this->onStateChange(this->getState());
        }
    }

    void applyPatch(unsigned const char *bytes, int offset, int length) {
        this->serializer->patch(bytes, offset, length);

        if (onStateChange) {
            this->onStateChange(this->getState());
        }
    }

    std::string getMessageHandlerKey(int32_t type) {
        return "i" + std::to_string(type);
    }

    std::string getMessageHandlerKey(std::string type) {
        return type;
    }

    // std::string getMessageHandlerKey (Schema type)
    // {
    //     return "s" + ?;
    // }

    Serializer<S> *serializer;
};
