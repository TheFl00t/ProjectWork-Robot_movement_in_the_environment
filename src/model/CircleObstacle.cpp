#include "CircleObstacle.h"

CircleObstacle::CircleObstacle(glm::vec2 pos, float radius)
    : Obstacle(pos),
      radius(radius)
{
    initCircle(radius);

    style.mode = DrawMode::FillAndOutline;
    style.fillColor = glm::vec4(0.5f, 0.5f, 0.5f, 0.4f);
    style.outlineColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); 
    style.lineWidth = 1.5f;
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
    radius = std::clamp(calculatedRadius, 5.0f, 1000.0f);
    updateMesh();
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