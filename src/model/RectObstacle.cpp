#include "RectObstacle.h"

RectObstacle::RectObstacle(glm::vec2 pos, float width, float height)
    : Obstacle(pos),
      width(width),
      height(height)
{
    initRect(width, height);
}

RectObstacle::~RectObstacle() {
    delete mesh;
}

void RectObstacle::updateMesh() {
    if (mesh) {
        delete mesh;
        initRect(width, height);
    }
}

void RectObstacle::update(float dt) {
    // ~
}

bool RectObstacle::checkCollision(Robot* robot) {
    // Знаходимо найближчу точку на прямокутнику до центру кола
    float closestX = std::clamp(robot->entityPos.x, entityPos.x, entityPos.x + width);
    float closestY = std::clamp(robot->entityPos.y, entityPos.y, entityPos.y + height);

    // Вектор від найближчої точки до центру кола
    float dx = robot->entityPos.x - closestX;
    float dy = robot->entityPos.y - closestY;

    float distSq = dx * dx + dy * dy;
    return (distSq <= robot->radius * robot->radius);
}

glm::vec2 RectObstacle::getCollisionPoint(Robot* robot) {
    // Знаходимо найближчу точку на прямокутнику до центру кола
    float closestX = std::clamp(robot->entityPos.x, entityPos.x, entityPos.x + width);
    float closestY = std::clamp(robot->entityPos.y, entityPos.y, entityPos.y + height);

    return glm::vec2(closestX, closestY);
}