#include "Environment.h"
#include "../constants.h"
#include "../view/Renderer.h"
#include <algorithm>

Environment::Environment(glm::vec2 pos, float width, float height) 
    : Entity(pos), width(width), height(height) 
{
    style.mode = DrawMode::Outline;
    style.outlineColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    style.lineWidth = 3.0f;
}

Environment::~Environment() {}

void Environment::addObstacle(std::unique_ptr<Obstacle> obs) {
    obstacles.push_back(std::move(obs));
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

bool Environment::getBounds(glm::vec2& outMin, glm::vec2& outMax) const {
    outMin = entityPos;
    outMax = entityPos + glm::vec2(width, height);
    return true;
}

void Environment::resizeByGizmo(const glm::vec2& mousePos) {
    float calculatedW = mousePos.x - entityPos.x;
    float calculatedH = mousePos.y - entityPos.y;
    setDimensions(calculatedW, calculatedH);
}

void Environment::drawVisitor(Renderer* renderer) {
    renderer->drawEnvironment(this);
}

void Environment::setDimensions(float w, float h) {
    width  = std::clamp(w, ARENA_SIZE_MIN, ARENA_SIZE_MAX);
    height = std::clamp(h, ARENA_SIZE_MIN, ARENA_SIZE_MAX);
}

float Environment::intersectRay(const glm::vec2& rayStart, const glm::vec2& rayDir) const {
    float minX = entityPos.x;
    float maxX = entityPos.x + width;
    float minY = entityPos.y;
    float maxY = entityPos.y + height;

    float invX = (std::abs(rayDir.x) > 1e-7f) ? 1.0f / rayDir.x : (rayDir.x >= 0.0f ? 1e7f : -1e7f);
    float invY = (std::abs(rayDir.y) > 1e-7f) ? 1.0f / rayDir.y : (rayDir.y >= 0.0f ? 1e7f : -1e7f);

    float txmin = (minX - rayStart.x) * invX;
    float txmax = (maxX - rayStart.x) * invX;
    if (txmin > txmax) std::swap(txmin, txmax);

    float tymin = (minY - rayStart.y) * invY;
    float tymax = (maxY - rayStart.y) * invY;
    if (tymin > tymax) std::swap(tymin, tymax);

    if ((txmin > tymax) || (tymin > txmax)) return -1.0f;

    float tmin = std::max(txmin, tymin);
    float tmax = std::min(txmax, tymax);

    if (tmax < 0.0f) return -1.0f;
    if (tmin < 0.0f) return tmax; 

    return tmin;
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
    for (const auto& obs : obstacles) {
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

void Environment::removeObstacle(Obstacle* obs) {
    auto it = std::find_if(obstacles.begin(), obstacles.end(), [obs](const auto& o) {
        return o.get() == obs;
    });
    
    if (it != obstacles.end()) {
        obstacles.erase(it);
    }
}