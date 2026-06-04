#pragma once

#include "Obstacle.h"
#include "../view/RectMesh.h"
#include "Robot.h"

class RectObstacle : public Obstacle {
public:
    float width;
    float height;

    RectObstacle(glm::vec2 pos, float width, float height);
    ~RectObstacle();

    void updateMesh() override;
    void update(float dt) override;

    // Новый полиморфный метод расчета коллизии
    CollisionInfo checkCollisionResult(Robot* robot) override;
    bool containsPoint(glm::vec2 point) override;
};