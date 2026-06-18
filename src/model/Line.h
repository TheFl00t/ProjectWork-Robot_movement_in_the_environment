#pragma once

#include "Entity.h"

class Line : public Entity {
public:
    glm::vec2 ptStart;
    glm::vec2 ptEnd;

    Line();
    ~Line();

    void update(float dt) override {}
    void drawVisitor(class Renderer* renderer) override;

    void setPoints(glm::vec2 start, glm::vec2 end);
};