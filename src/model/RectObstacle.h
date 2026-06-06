#pragma once

#include "Obstacle.h"
#include "Robot.h"

class RectObstacle : public Obstacle {
private:
    float width;
    float height;

public:
    RectObstacle(glm::vec2 pos, float width, float height);
    ~RectObstacle();

    float getWidth() const { return width; }
    float getHeight() const { return height; }
    void setDimensions(float w, float h);

    void update(float dt) override;
    bool containsPoint(glm::vec2 point) override;
    void drawVisitor(class Renderer* renderer) override;

    bool getBounds(glm::vec2& outMin, glm::vec2& outMax) const override;
    void resizeByGizmo(const glm::vec2& mousePos) override;

    // Новый полиморфный метод расчета коллизии
    CollisionInfo checkCollisionResult(Robot* robot) override;

    std::string getTypeName() const override { return "Rectangle"; }
    void serialize(json& j) const override;
};