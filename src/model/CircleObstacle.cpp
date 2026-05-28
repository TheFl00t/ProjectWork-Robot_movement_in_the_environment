#include "CircleObstacle.h"

CircleObstacle::CircleObstacle(glm::vec2 pos, float radius)
    : Obstacle(pos),
      radius(radius)
{
    initCircle(radius);
}

CircleObstacle::~CircleObstacle() {
    delete mesh;
}

void CircleObstacle::updateMesh() {
    if (mesh) {
        delete mesh;
        initCircle(radius);
    }
}

void CircleObstacle::update(float dt) {
    // ~
}

CollisionInfo CircleObstacle::checkCollisionResult(Robot* robot) {
    CollisionInfo info;

    glm::vec2 dir = robot->entityPos - this->entityPos;
    float dist = glm::length(dir);
    float minDist = robot->radius + this->radius;

    if (dist < minDist) {
        info.collided = true;
        // Направление от центра препятствия к роботу. Если центры совпали, выталкиваем вверх
        info.normal = (dist > 0.0f) ? dir / dist : glm::vec2(0.0f, -1.0f);
        info.depth = minDist - dist;
        info.contactPoint = this->entityPos + info.normal * this->radius;
    }

    return info;
}