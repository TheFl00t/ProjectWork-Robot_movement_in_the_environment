#pragma once

#include "../model/NavGrid.h"
#include <queue>

struct GridNode {
    int col, row;
    float gScore; // Вартість від старту
    float fScore; // Фактична вартість + евристика

    bool operator>(const GridNode& other) const {
        return fScore > other.fScore;
    }
};

class Pathfinder {
private:
    static float heuristic(int c1, int r1, int c2, int r2);

public:
    static std::vector<glm::vec2> findPath(const NavGrid& grid, const glm::vec2& startPos, const glm::vec2& targetPos, const glm::vec2& arenaPos);
};