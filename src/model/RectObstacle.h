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

    // Перевірка на зіткнення (AABB - Коло)
    bool checkCollision(Robot* robot) override;

    // Знаходження найближчої точки на прямокутнику
    glm::vec2 getCollisionPoint(Robot* robot);
};