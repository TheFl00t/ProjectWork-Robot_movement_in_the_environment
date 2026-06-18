#pragma once

#include "Entity.h"

class TargetPoint : public Entity {
private:
    float clickRadius = 15.0f; // Радіус захоплення цілі мишкою

public:
    bool active = false;
    bool isDragging = false;   // Флаг утримання лівої кнопки миші на цілі
    bool isMovingTo = false;   // Дозвіл роботу їхати

    TargetPoint(glm::vec2 pos = glm::vec2(0.0f)) : Entity(pos) {
        style.mode = DrawMode::Outline;
        style.outlineColor = glm::vec4(1.0f, 0.6f, 0.0f, 1.0f);
        style.lineWidth = 2.0f;
    }

    ~TargetPoint() override = default;

    void update(float dt) override {}

    bool containsPoint(glm::vec2 point) override {
        return active && glm::distance(point, entityPos) < clickRadius;
    }

    void drawVisitor(class Renderer* renderer) override;
};