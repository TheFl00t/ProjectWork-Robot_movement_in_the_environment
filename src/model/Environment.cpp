#include "Environment.h"

Environment::Environment(glm::vec2 pos, float width, float height) 
    : Entity(pos), width(width), height(height) 
{
    setMesh(new RectMesh(width, height, false));

    style.mode = DrawMode::Outline;
    style.outlineColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    style.lineWidth = 3.0f;
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
    // Середовище статичне
}

bool Environment::containsPoint(glm::vec2 point) {
    float left   = entityPos.x;
    float right  = entityPos.x + width;
    float top    = entityPos.y;
    float bottom = entityPos.y + height;
    float thickness = 8.0f;

    bool nearLeft   = (point.x >= left - thickness && point.x <= left + thickness && point.y >= top && point.y <= bottom);
    bool nearRight  = (point.x >= right - thickness && point.x <= right + thickness && point.y >= top && point.y <= bottom);
    bool nearTop    = (point.y >= top - thickness && point.y <= top + thickness && point.x >= left && point.x <= right);
    bool nearBottom = (point.y >= bottom - thickness && point.y <= bottom + thickness && point.x >= left && point.x <= right);

    return (nearLeft || nearRight || nearTop || nearBottom);
}

void Environment::removeObstacle(Obstacle* obs) {
    auto it = std::find(obstacles.begin(), obstacles.end(), obs);
    if (it != obstacles.end()) {
        delete *it; // Викликається деструктор Obstacle, який видалить свій mesh
        obstacles.erase(it);
    }
}

CollisionInfo Environment::checkCollisionResult(Robot* robot) {
    CollisionInfo info;

    float left   = entityPos.x;
    float right  = entityPos.x + width;
    float top    = entityPos.y;
    float bottom = entityPos.y + height;

    // 1. Перевірка зовнішніх меж арени (виштовхування всередину)
    if (robot->entityPos.x - robot->getRadius() < left) {
        info.collided = true;
        info.depth = left - (robot->entityPos.x - robot->getRadius());
        info.normal = glm::vec2(1.0f, 0.0f);
        info.contactPoint = glm::vec2(left, robot->entityPos.y);
        return info;
    }
    if (robot->entityPos.x + robot->getRadius() > right) {
        info.collided = true;
        info.depth = (robot->entityPos.x + robot->getRadius()) - right;
        info.normal = glm::vec2(-1.0f, 0.0f);
        info.contactPoint = glm::vec2(right, robot->entityPos.y);
        return info;
    }
    if (robot->entityPos.y - robot->getRadius() < top) {
        info.collided = true;
        info.depth = top - (robot->entityPos.y - robot->getRadius());
        info.normal = glm::vec2(0.0f, 1.0f);
        info.contactPoint = glm::vec2(robot->entityPos.x, top);
        return info;
    }
    if (robot->entityPos.y + robot->getRadius() > bottom) {
        info.collided = true;
        info.depth = (robot->entityPos.y + robot->getRadius()) - bottom;
        info.normal = glm::vec2(0.0f, -1.0f);
        info.contactPoint = glm::vec2(robot->entityPos.x, bottom);
        return info;
    }

    // 2. Перевірка внутрішніх перешкод на карті
    for (auto* obs : obstacles) {
        CollisionInfo obsInfo = obs->checkCollisionResult(robot);
        if (obsInfo.collided) {
            // Якщо зачеплено кілька перешкод, вибираємо ту, де проникнення найглибше
            if (!info.collided || obsInfo.depth > info.depth) {
                info = obsInfo;
            }
        }
    }

    return info;
}