#pragma once

#include "Entity.h"
#include <vector>

class PathEntity : public Entity {
public:
    std::vector<glm::vec2> waypoints;
    int renderType = 0; // 0 = Line, 1 = Dots, 2 = Squares
    float elementSize = 20.0f;

    // межі для обрізання squere точок
    glm::vec2 arenaPos = glm::vec2(0.0f);
    float arenaWidth = 0.0f;
    float arenaHeight = 0.0f;

    PathEntity() : Entity(glm::vec2(0.0f)) {
        style.mode = DrawMode::Outline;
        style.outlineColor = glm::vec4(1.0f, 0.6f, 0.0f, 1.0f);
        style.lineWidth = 2.5f;
    }

    ~PathEntity() override = default;

    void update(float dt) override {}

    void drawVisitor(class Renderer* renderer) override;
};