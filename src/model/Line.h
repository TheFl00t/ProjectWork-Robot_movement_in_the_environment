#pragma once

#include "Entity.h"
#include "../view/LineMesh.h"

class Line : public Entity {
private:
    LineMesh* lineMesh = nullptr;

public:
    Line();
    ~Line();

    void update(float dt) override {}

    void setPoints(glm::vec2 start, glm::vec2 end);
};