#pragma once

#include "Entity.h"
#include "../view/CircleMesh.h"

class Robot : public Entity {
private:
    float radius;
    float velocity;
    CircleMesh* circleMesh = nullptr;

public:
    glm::vec2 direction;
    
    // Значення для функції Reset
    glm::vec2 startPos;
    float startRadius;
    float startVelocity;

    Robot(glm::vec2 pos, float radius, float velocity);
    ~Robot();

    void update(float dt) override;
    bool containsPoint(glm::vec2 point) override;

    bool getBounds(glm::vec2& outMin, glm::vec2& outMax) const override;
    void resizeByGizmo(const glm::vec2& mousePos) override;

    float getRadius() const { return radius; }
    float getVelocity() const { return velocity; }

    void setRadius(float newRadius);
    void setVelocity(float newVelocity);
};