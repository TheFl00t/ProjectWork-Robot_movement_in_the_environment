#pragma once

#include "../config.h"
#include "Environment.h"

class NavGrid {
public:
    int rows = 0;
    int cols = 0;
    float cellSize = 20.0f;
    std::vector<std::vector<bool>> blocked;

    NavGrid() = default;
    ~NavGrid() = default;

    // Скан сцени та побудова карти прохідності з урахуванням радіуса робота
    void bake(const Environment& env, float robotRadius, const class Robot* robot = nullptr, bool useLidarMode = false, bool forceClear = false);
    
    // Конвертація координат
    glm::vec2 cellToWorld(int col, int row, const glm::vec2& arenaPos) const;
    bool worldToCell(const glm::vec2& worldPos, const glm::vec2& arenaPos, int& outCol, int& outRow) const;
    bool isBlocked(int col, int row) const;
};