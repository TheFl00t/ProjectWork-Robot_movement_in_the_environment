#include "RectObstacle.h"

RectObstacle::RectObstacle(glm::vec2 pos, float width, float height)
    : Obstacle(pos),
      width(width),
      height(height)
{
    initRect(width, height);

    style.mode = DrawMode::FillAndOutline;
    style.fillColor = glm::vec4(0.5f, 0.5f, 0.5f, 0.4f);
    style.outlineColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); 
    style.lineWidth = 1.5f;
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
    // Прямокутна перешкода статична
}

bool RectObstacle::containsPoint(glm::vec2 point) {
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    
    return (point.x >= entityPos.x - hw && point.x <= entityPos.x + hw &&
            point.y >= entityPos.y - hh && point.y <= entityPos.y + hh);
}

bool RectObstacle::getBounds(glm::vec2& outMin, glm::vec2& outMax) const {
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    outMin = entityPos - glm::vec2(hw, hh);
    outMax = entityPos + glm::vec2(hw, hh);
    return true;
}

void RectObstacle::resizeByGizmo(const glm::vec2& mousePos) {
    float calculatedW = (mousePos.x - entityPos.x) * 2.0f;
    float calculatedH = (mousePos.y - entityPos.y) * 2.0f;
    width = std::clamp(calculatedW, 10.0f, 1000.0f);
    height = std::clamp(calculatedH, 10.0f, 1000.0f);
    updateMesh();
}

CollisionInfo RectObstacle::checkCollisionResult(Robot* robot) {
    CollisionInfo info;

    // Обчислюємо межі прямокутника на основі центру (entityPos) та напіврозмірів
    float hw = width * 0.5f;
    float hh = height * 0.5f;
    
    float left   = entityPos.x - hw;
    float right  = entityPos.x + hw;
    float top    = entityPos.y - hh;
    float bottom = entityPos.y + hh;

    // Знаходимо найближчу точку на прямокутнику до центру робота з урахуванням нових меж
    float closestX = std::clamp(robot->entityPos.x, left, right);
    float closestY = std::clamp(robot->entityPos.y, top, bottom);
    glm::vec2 closestPoint(closestX, closestY);

    glm::vec2 dir = robot->entityPos - closestPoint;
    float dist = glm::length(dir);

    // Центр робота зовні або на межі прямокутника
    if (dist > 0.0f) {
        if (dist < robot->getRadius()) {
            info.collided = true;
            info.normal = dir / dist;
            info.depth = robot->getRadius() - dist;
            info.contactPoint = closestPoint;
        }
    } 
    // Центр робота опинився глибоко всередині прямокутника (dist == 0)
    else {
        info.collided = true;
        // Обчислюємо відстані до всіх 4 граней від центра
        float dl = robot->entityPos.x - left;
        float dr = right - robot->entityPos.x;
        float d_top = robot->entityPos.y - top;
        float d_bottom = bottom - robot->entityPos.y;

        float minDist = std::min({dl, dr, d_top, d_bottom});

        if (minDist == dl) {
            info.normal = glm::vec2(-1.0f, 0.0f);
            info.contactPoint = glm::vec2(left, robot->entityPos.y);
        } else if (minDist == dr) {
            info.normal = glm::vec2(1.0f, 0.0f);
            info.contactPoint = glm::vec2(right, robot->entityPos.y);
        } else if (minDist == d_top) {
            info.normal = glm::vec2(0.0f, -1.0f);
            info.contactPoint = glm::vec2(robot->entityPos.x, top);
        } else {
            info.normal = glm::vec2(0.0f, 1.0f);
            info.contactPoint = glm::vec2(robot->entityPos.x, bottom);
        }
        info.depth = robot->getRadius() + minDist;
    }

    return info;
}

void RectObstacle::serialize(json& j) const {
    j["type"] = "rect";
    j["width"] = this->width;
    j["height"] = this->height;
}