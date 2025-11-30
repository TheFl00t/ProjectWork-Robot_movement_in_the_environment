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

bool CircleObstacle::checkCollision(Robot* robot) {
    // Відстань між центрами кіл
    glm::vec2 diff = robot->entityPos - this->entityPos;

    float distSq = diff.x * diff.x + diff.y * diff.y;
    float minDist = robot->radius + this->radius;

    return (distSq <= minDist * minDist);
}

glm::vec2 CircleObstacle::getCollisionPoint(Robot* robot) {
    glm::vec2 dir = robot->entityPos - entityPos;
    float dist = glm::length(dir);

    if (dist == 0.0f) return entityPos;

    glm::vec2 normDir = dir / dist;

    // Точка на границі перешкоди у напрямку до робота
    glm::vec2 contactPoint = entityPos + normDir * radius;
    return contactPoint;
}