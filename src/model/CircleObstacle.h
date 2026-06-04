#pragma once

#include "Obstacle.h"
#include "../view/CircleMesh.h"
#include "Robot.h"

class CircleObstacle : public Obstacle {
public:
    float radius;

    CircleObstacle(glm::vec2 pos, float radius);
    ~CircleObstacle();

    void updateMesh() override;
    void update(float dt) override;
    bool containsPoint(glm::vec2 point) override;

    // Новый полиморфный метод расчета коллизии
    CollisionInfo checkCollisionResult(Robot* robot) override;
};