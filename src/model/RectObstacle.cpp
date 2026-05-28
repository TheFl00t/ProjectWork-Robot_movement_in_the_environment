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

CollisionInfo RectObstacle::checkCollisionResult(Robot* robot) {
    CollisionInfo info;

    // Находим ближайшую точку на прямоугольнике к центру робота
    float closestX = std::clamp(robot->entityPos.x, entityPos.x, entityPos.x + width);
    float closestY = std::clamp(robot->entityPos.y, entityPos.y, entityPos.y + height);
    glm::vec2 closestPoint(closestX, closestY);

    glm::vec2 dir = robot->entityPos - closestPoint;
    float dist = glm::length(dir);

    // Центр робота снаружи или на границе прямоугольника
    if (dist > 0.0f) {
        if (dist < robot->radius) {
            info.collided = true;
            info.normal = dir / dist;
            info.depth = robot->radius - dist;
            info.contactPoint = closestPoint;
        }
    } 
    // Центр робота оказался глубоко внутри прямоугольника (dist == 0)
    else {
        info.collided = true;
        // Вычисляем расстояния до всех 4 граней для определения кратчайшего пути выталкивания
        float dl = robot->entityPos.x - entityPos.x;
        float dr = (entityPos.x + width) - robot->entityPos.x;
        float dt = robot->entityPos.y - entityPos.y;
        float db = (entityPos.y + height) - robot->entityPos.y;

        float minDist = std::min({dl, dr, dt, db});

        if (minDist == dl) {
            info.normal = glm::vec2(-1.0f, 0.0f);
            info.contactPoint = glm::vec2(entityPos.x, robot->entityPos.y);
        } else if (minDist == dr) {
            info.normal = glm::vec2(1.0f, 0.0f);
            info.contactPoint = glm::vec2(entityPos.x + width, robot->entityPos.y);
        } else if (minDist == dt) {
            info.normal = glm::vec2(0.0f, -1.0f);
            info.contactPoint = glm::vec2(robot->entityPos.x, entityPos.y);
        } else {
            info.normal = glm::vec2(0.0f, 1.0f);
            info.contactPoint = glm::vec2(robot->entityPos.x, entityPos.y + height);
        }
        info.depth = robot->radius + minDist;
    }

    return info;
}