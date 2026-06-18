#pragma once

#include "Entity.h"
#include "Lidar.h"

class Robot : public Entity {
private:
    float radius;
    float velocity;
    float angle = 0.0f;
    Lidar lidar;

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
    void drawVisitor(class Renderer* renderer) override;

    Lidar& getLidar() { return lidar; }
    const Lidar& getLidar() const { return lidar; }

    float getRadius() const { return radius; }
    float getVelocity() const { return velocity; }
    float getAngle() const { return angle; }
    
    void setRadius(float newRadius);
    void setVelocity(float newVelocity);
    void setAngle(float a) { angle = a; }
};