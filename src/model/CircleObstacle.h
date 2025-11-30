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

    // Перевірка на зіткнення з роботом (Коло-Коло)
    bool checkCollision(Robot* robot) override;
    
    // Розрахунок точки дотику
    glm::vec2 getCollisionPoint(Robot* robot);
};