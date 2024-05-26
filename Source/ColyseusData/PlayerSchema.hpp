#pragma once

#include "Colyseus/Serializer/schema.h"

using namespace colyseus::schema;

class PlayerSchema : public Schema {
public:
    float x = 0;
    float y = 0;

    PlayerSchema() {
        this->_indexes = {{0, "x"}, {1, "y"}};
        this->_types = {{0, "float32"}, {1, "float32"}};
    }
};
