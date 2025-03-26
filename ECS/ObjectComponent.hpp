#pragma once

#include "ECS.hpp"
#include <string>

class ObjectComponent : public Component {
public:
    std::string type;

    ObjectComponent(const std::string& objectType) : type(objectType) {}
    ~ObjectComponent() {}
}; 