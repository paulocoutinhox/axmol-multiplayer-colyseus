#pragma once

#include <string>
#include <functional>
#include <vector>
#include "thirdparty/nlohmann/json.hpp"
#include "network/WebSocket.h"
#include "network/HttpClient.h"
#include "Room.hpp"
#include "Connection.hpp"

using namespace ax::network;
using namespace nlohmann;

typedef json JoinOptions;

class MatchMakeError
{
public:
    MatchMakeError(int _code, std::string _message)
        : code(_code), message(std::move(_message)) {}

    int code;
    std::string message;
};

class Client : public ax::Object
{
public:
    std::string endpoint;

    Client(std::string endpoint)
        : endpoint(std::move(endpoint)) {}

    template <typename S>
    void joinOrCreate(const std::string& roomName, const JoinOptions& options, std::function<void(MatchMakeError*, Room<S>*)> callback)
    {
        createMatchMakeRequest<S>("joinOrCreate", roomName, options, callback);
    }

    template <typename S>
    void join(const std::string& roomName, const JoinOptions& options, std::function<void(MatchMakeError*, Room<S>*)> callback)
    {
        createMatchMakeRequest<S>("join", roomName, options, callback);
    }

    template <typename S>
    void create(const std::string& roomName, const JoinOptions& options, std::function<void(MatchMakeError*, Room<S>*)> callback)
    {
        createMatchMakeRequest<S>("create", roomName, options, callback);
    }

    template <typename S>
    void joinById(const std::string& roomId, const JoinOptions& options, std::function<void(MatchMakeError*, Room<S>*)> callback)
    {
        createMatchMakeRequest<S>("joinById", roomId, options, callback);
    }

    template <typename S>
    void reconnect(const std::string& roomId, const std::string& sessionId, std::function<void(MatchMakeError*, Room<S>*)> callback)
    {
        createMatchMakeRequest<S>("joinById", roomId, {{"sessionId", sessionId}}, callback);
    }

private:
    template <typename S>
    void createMatchMakeRequest(const std::string& method, const std::string& roomName, const JoinOptions& options, std::function<void(MatchMakeError*, Room<S>*)> callback)
    {
        auto req = new (std::nothrow) HttpRequest();

        // replace "ws" with "http" if it appears at the start of the endpoint
        std::string url = this->endpoint;
        if (url.rfind("ws", 0) == 0) {
            url.replace(0, 2, "http");
        }
        url += "/matchmake/" + method + "/" + roomName;
        req->setUrl(url);
        req->setRequestType(HttpRequest::Type::POST);

        std::vector<std::string> headers = {"Accept:application/json", "Content-Type:application/json"};
        req->setHeaders(headers);

        std::string data = options.dump();
        if (data == "null") {
            data = "{}";
        }
        req->setRequestData(data.c_str(), data.length());

        req->setResponseCallback([this, roomName, callback](HttpClient* client, HttpResponse* response) {
            if (!response || !response->isSucceed()) {
                MatchMakeError* error = new MatchMakeError(response ? static_cast<int>(response->getResponseCode()) : 0, "server error");
                callback(error, nullptr);
                delete error;
                return;
            }

            // handle response data using yasio::sbyte_buffer
            yasio::sbyte_buffer* responseData = response->getResponseData();
            std::string json_string(responseData->data(), responseData->data() + responseData->size());
            auto json = nlohmann::json::parse(json_string);

            // server responded with error
            if (json.contains("error") && json["error"].is_string()) {
                MatchMakeError* error = new MatchMakeError(json["code"].get<int>(), json["error"].get<std::string>());
                callback(error, nullptr);
                delete error;
                return;
            }

            auto room = new Room<S>(roomName);
            room->id = json["room"]["roomId"].get<std::string>();
            room->sessionId = json["sessionId"].get<std::string>();

            std::string processId = json["room"]["processId"].get<std::string>();

            room->onError = [callback](const int& code, const std::string& message) {
                MatchMakeError* error = new MatchMakeError(code, message);
                callback(error, nullptr);
                delete error;
            };

            room->onJoin = [room, callback]() {
                room->onError = nullptr;
                callback(nullptr, room);
            };

            room->connect(this->createConnection(processId + "/" + room->id + "?sessionId=" + room->sessionId));
        });

        HttpClient::getInstance()->send(req);
        req->release();
    }

    Connection* createConnection(const std::string& path, const JoinOptions& options = JoinOptions())
    {
        return new Connection(this->endpoint + "/" + path);
    }
};
