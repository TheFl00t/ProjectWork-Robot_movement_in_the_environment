#pragma once

#include "Obstacle.h"
#include "Robot.h"

class CircleObstacle : public Obstacle {
private:
    float radius;

public:
    CircleObstacle(glm::vec2 pos, float radius);
    ~CircleObstacle();

    void update(float dt) override;
    bool containsPoint(glm::vec2 point) override;

    bool getBounds(glm::vec2& outMin, glm::vec2& outMax) const override;
    void resizeByGizmo(const glm::vec2& mousePos) override;
    void drawVisitor(class Renderer* renderer) override;

    // Новый полиморфный метод расчета коллизии
    CollisionInfo checkCollisionResult(Robot* robot) override;

    std::string getTypeName() const override { return "Circle"; }
    void serialize(json& j) const override;

    float getRadius() const { return radius; }
    void setRadius(float newRadius);
};