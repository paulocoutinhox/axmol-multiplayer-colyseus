#pragma once

#include "PlayerSchema.hpp"
#include "schema.h"
#include <typeindex>
#include <typeinfo>

using namespace colyseus::schema;

class RoomStateSchema : public Schema {
public:
    MapSchema<PlayerSchema *> *players = new MapSchema<PlayerSchema *>();

    RoomStateSchema() {
        this->_indexes = {{0, "players"}};
        this->_types = {{0, "map"}};
        this->_childPrimitiveTypes = {};
        this->_childSchemaTypes = {{0, typeid(PlayerSchema)}};
    }

    virtual ~RoomStateSchema() {
        delete this->players;
    }

protected:
    inline MapSchema<char *> *getMap(const std::string &field) {
        if (field == "players") {
            return (MapSchema<char *> *)this->players;
        }
        return Schema::getMap(field);
    }

    inline void setMap(const std::string &field, MapSchema<char *> *value) {
        if (field == "players") {
            this->players = (MapSchema<PlayerSchema *> *)value;
            return;
        }
        return Schema::setMap(field, value);
    }

    inline Schema *createInstance(std::type_index type) {
        if (type == typeid(PlayerSchema)) {
            return new PlayerSchema();
        }
        return Schema::createInstance(type);
    }
};
