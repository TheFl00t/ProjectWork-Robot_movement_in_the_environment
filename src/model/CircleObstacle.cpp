#include "CircleObstacle.h"
#include "../constants.h"
#include "../view/Renderer.h"

CircleObstacle::CircleObstacle(glm::vec2 pos, float radius)
    : Obstacle(pos),
      radius(radius)
{
    style.mode = DrawMode::FillAndOutline;
    style.fillColor = glm::vec4(0.5f, 0.5f, 0.5f, 0.4f);
    style.outlineColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); 
    style.lineWidth = 1.5f;
}

CircleObstacle::~CircleObstacle() {}

void CircleObstacle::update(float dt) {
    // ~
}

bool CircleObstacle::containsPoint(glm::vec2 point) {
    glm::vec2 dir = point - entityPos;
    return glm::dot(dir, dir) <= (radius * radius);
}

bool CircleObstacle::getBounds(glm::vec2& outMin, glm::vec2& outMax) const {
    outMin = entityPos - glm::vec2(radius);
    outMax = entityPos + glm::vec2(radius);
    return true;
}

void CircleObstacle::resizeByGizmo(const glm::vec2& mousePos) {
    float calculatedRadius = mousePos.x - entityPos.x;
    setRadius(calculatedRadius);
}

void CircleObstacle::drawVisitor(Renderer* renderer) {
    renderer->drawCircleObstacle(this);
}

float CircleObstacle::intersectRay(const glm::vec2& rayStart, const glm::vec2& rayDir) const {
    glm::vec2 v = rayStart - entityPos;
    float b = 2.0f * glm::dot(rayDir, v);
    float c = glm::dot(v, v) - radius * radius;
    float discriminant = b * b - 4.0f * c;

    if (discriminant < 0.0f) return -1.0f;

    float t1 = (-b - std::sqrt(discriminant)) / 2.0f;
    if (t1 > 0.0f) return t1;

    float t2 = (-b + std::sqrt(discriminant)) / 2.0f;
    if (t2 > 0.0f) return t2;

    return -1.0f;
}

CollisionInfo CircleObstacle::checkCollisionResult(Robot* robot) {
    CollisionInfo info;

    glm::vec2 dir = robot->entityPos - this->entityPos;
    float dist = glm::length(dir);
    float minDist = robot->getRadius() + this->radius;

    if (dist < minDist) {
        info.collided = true;
        // Напрямок від центру перешкоди до робота. Якщо центри збіглися, виштовхуємо вгору
        info.normal = (dist > 0.0f) ? dir / dist : glm::vec2(0.0f, -1.0f);
        info.depth = minDist - dist;
        info.contactPoint = this->entityPos + info.normal * this->radius;
    }

    return info;
}

void CircleObstacle::serialize(json& j) const {
    j["type"] = "circle";
    j["radius"] = this->radius;
}

void CircleObstacle::setRadius(float newRadius) {
    radius = std::clamp(newRadius, CIRCLE_RADIUS_MIN, CIRCLE_RADIUS_MAX);
}