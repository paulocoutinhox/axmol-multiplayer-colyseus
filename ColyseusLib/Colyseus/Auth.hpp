#pragma once

#include <stdio.h>
#include <vector>

#include "axmol.h"
#include "network/HttpClient.h"

using namespace ax::network;

class Auth : public ax::Object
{
public:
    Auth(std::string endpoint);
    virtual ~Auth();

    // Properties
    std::string token;

    std::string _id;
    std::string username;
    std::string displayName;
    std::string avatarUrl;

    bool isAnonymous;
    std::string email;

    std::string lang;
    std::string location;
    std::string timezone;
    // any metadata; // Json::Value

    // Device devices;

    std::string facebookId;
    std::string twitterId;
    std::string googleId;
    std::string gameCenterId;
    std::string steamId;

    std::string friendIds;
    std::string blockedUserIds;

    std::string createdAt;
    std::string updatedAt;

    // Methods
    void login();
    void login(std::string email, std::string password);
    void login(std::string fbAccessToken);
    void login(std::vector<std::string> query);

    void save();

    void logout();

protected:
    std::string endpoint;

    void request(HttpRequest::Type method, std::string segments, std::vector<std::string> query);
    void request(HttpRequest::Type method, std::string segments, std::vector<std::string> query, std::string upload);
    void onHttpRequestCompleted(HttpClient * client, HttpResponse * response);

    std::string getPlatform();
    std::string getDeviceId();

    std::string readToken();
    void writeToken(std::string token);

    std::string urlencode(const std::string &value);
    std::string string_join(const std::vector<std::string> &elements, const std::string &separator);
};
