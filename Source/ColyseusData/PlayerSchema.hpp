#pragma once

#include "Colyseus/Serializer/schema.h"

using namespace colyseus::schema;

class PositionSchema : public Schema {
public:
    float x = 0;
    float y = 0;

    PositionSchema() {
        this->_indexes = {{0, "x"}, {1, "y"}};
        this->_types = {{0, "float32"}, {1, "float32"}};
    }
};

class PlayerSchema : public Schema {
public:
    std::string name;
    PositionSchema *position;

    PlayerSchema() {
        this->position = new PositionSchema();
        this->_indexes = {{0, "name"}, {1, "position"}};
        this->_types = {{0, "string"}, {1, "ref"}};
        this->_childPrimitiveTypes = {};
        this->_childSchemaTypes = {{1, typeid(PositionSchema)}};
    }

    virtual ~PlayerSchema() {
        delete this->position;
    }
};
