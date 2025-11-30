#pragma once

#include "Entity.h"
#include "../view/CircleMesh.h"

class Robot : public Entity {
public:
    float radius;
    float velocity;
    glm::vec2 direction;
    
    // Значення для функції Reset
    glm::vec2 startPos;
    float startRadius;
    float startVelocity;

    Robot(glm::vec2 pos, float radius, float velocity);
    ~Robot();

    void update(float dt) override;
};