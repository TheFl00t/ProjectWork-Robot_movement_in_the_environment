#include "Environment.h"

Environment::Environment(glm::vec2 pos, float width, float height) :
    Entity(pos), width(width), height(height) 
{
    setMesh(new RectMesh(width, height));
}

Environment::~Environment() {
    for (auto obs : obstacles) {
        delete obs;
    }
    delete mesh;
}

void Environment::addObstacle(Obstacle* obs) {
    obstacles.push_back(obs);
}

void Environment::update(float dt) {
    // ~
}

CollisionInfo Environment::checkCollisionResult(Robot* robot) {
    CollisionInfo info;

    float left   = entityPos.x;
    float right  = entityPos.x + width;
    float top    = entityPos.y;
    float bottom = entityPos.y + height;

    // 1. Проверка внешних границ арены (выталкивание внутрь)
    if (robot->entityPos.x - robot->radius < left) {
        info.collided = true;
        info.depth = left - (robot->entityPos.x - robot->radius);
        info.normal = glm::vec2(1.0f, 0.0f);
        info.contactPoint = glm::vec2(left, robot->entityPos.y);
        return info;
    }
    if (robot->entityPos.x + robot->radius > right) {
        info.collided = true;
        info.depth = (robot->entityPos.x + robot->radius) - right;
        info.normal = glm::vec2(-1.0f, 0.0f);
        info.contactPoint = glm::vec2(right, robot->entityPos.y);
        return info;
    }
    if (robot->entityPos.y - robot->radius < top) {
        info.collided = true;
        info.depth = top - (robot->entityPos.y - robot->radius);
        info.normal = glm::vec2(0.0f, 1.0f);
        info.contactPoint = glm::vec2(robot->entityPos.x, top);
        return info;
    }
    if (robot->entityPos.y + robot->radius > bottom) {
        info.collided = true;
        info.depth = (robot->entityPos.y + robot->radius) - bottom;
        info.normal = glm::vec2(0.0f, -1.0f);
        info.contactPoint = glm::vec2(robot->entityPos.x, bottom);
        return info;
    }

    // 2. Проверка внутренних препятствий
    for (auto* obs : obstacles) {
        CollisionInfo obsInfo = obs->checkCollisionResult(robot);
        if (obsInfo.collided) {
            // Если задето несколько препятствий, выбираем то, где проникновение глубже
            if (!info.collided || obsInfo.depth > info.depth) {
                info = obsInfo;
            }
        }
    }

    return info;
}