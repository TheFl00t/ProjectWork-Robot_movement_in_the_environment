#pragma once

#include "Entity.h"

class Point : public Entity {
public:
    Point(glm::vec2 pos, glm::vec4 col);
    ~Point();

    void update(float dt) override;
    void drawVisitor(class Renderer* renderer) override;
    
    void setAlpha(float alpha);
};