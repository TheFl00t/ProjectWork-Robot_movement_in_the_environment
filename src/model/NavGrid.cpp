#include "NavGrid.h"
#include "../constants.h"
#include "CircleObstacle.h"
#include "RectObstacle.h"

void NavGrid::bake(const Environment& env, float robotRadius, const class Robot* robot, bool useLidar, bool forceClear) {
    int newCols = static_cast<int>(std::ceil(env.width / cellSize));
    int newRows = static_cast<int>(std::ceil(env.height / cellSize));

    // 1. ПЕРЕВІРКА НА ПОВНЕ ОЧИЩЕННЯ КАРТИ
    // Очищаємо сітку лише якщо змінилися розміри арени, вимкнули режим лідара або користувач натиснув кнопку скидання
    if (this->cols != newCols || this->rows != newRows || !useLidar || forceClear) {
        this->cols = newCols;
        this->rows = newRows;
        blocked.assign(cols, std::vector<bool>(rows, false));

        // Якщо ми повернулися у звичайний статичний режим (GSE), одразу запікаємо зовнішні стіни арени
        if (!useLidar) {
            const float safetyMargin = 3.0f;
            const float clearance = robotRadius + safetyMargin;
            for (int c = 0; c < cols; ++c) {
                for (int r = 0; r < rows; ++r) {
                    glm::vec2 cellCenter = cellToWorld(c, r, env.entityPos);
                    if (cellCenter.x - clearance < env.entityPos.x || 
                        cellCenter.x + clearance > env.entityPos.x + env.width ||
                        cellCenter.y - clearance < env.entityPos.y || 
                        cellCenter.y + clearance > env.entityPos.y + env.height) 
                    {
                        blocked[c][r] = true;
                    }
                }
            }
        }
    }

    const float safetyMargin = 3.0f;
    const float clearance = robotRadius + safetyMargin;

    // 2. РЕЖИМ ЛІДАРА (ДИНАМІЧНЕ НАКОПИЧЕННЯ)
    if (useLidar) {
        if (robot) {
            const auto& impactPoints = robot->getLidar().getImpactPoints();
            const std::vector<float>& scanData = robot->getLidar().getScanData();
            float maxLidarDist = robot->getLidar().getMaxDistance();

            for (size_t i = 0; i < impactPoints.size(); ++i) {
                if (scanData[i] >= maxLidarDist - 1e-3f) continue;

                int centerC, centerR;
                if (worldToCell(impactPoints[i], env.entityPos, centerC, centerR)) {
                    int cellRadius = static_cast<int>(std::ceil(clearance / cellSize));
                    for (int dc = -cellRadius; dc <= cellRadius; ++dc) {
                        for (int dr = -cellRadius; dr <= cellRadius; ++dr) {
                            int tc = centerC + dc;
                            int tr = centerR + dr;
                            
                            if (tc >= 0 && tc < cols && tr >= 0 && tr < rows) {
                                glm::vec2 cellCenter = cellToWorld(tc, tr, env.entityPos);
                                if (glm::distance(cellCenter, impactPoints[i]) < clearance) {
                                    blocked[tc][tr] = true; // Додаємо нову перешкоду поверх старих
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // 3. СТАТИЧНИЙ РЕЖИМ (GLOBAL STATIC ENV)
    else {
        for (int c = 0; c < cols; ++c) {
            for (int r = 0; r < rows; ++r) {
                if (blocked[c][r]) continue;
                glm::vec2 cellCenter = cellToWorld(c, r, env.entityPos);

                for (const auto& obs : env.getObstacles()) {
                    if (auto* circle = dynamic_cast<CircleObstacle*>(obs.get())) {
                        float dist = glm::distance(cellCenter, circle->entityPos);
                        if (dist < circle->getRadius() + clearance) {
                            blocked[c][r] = true;
                            break;
                        }
                    }
                    else if (auto* rect = dynamic_cast<RectObstacle*>(obs.get())) {
                        float hw = rect->getWidth() * 0.5f;
                        float hh = rect->getHeight() * 0.5f;
                        float cx = std::clamp(cellCenter.x, rect->entityPos.x - hw, rect->entityPos.x + hw);
                        float cy = std::clamp(cellCenter.y, rect->entityPos.y - hh, rect->entityPos.y + hh);
                        float dist = glm::distance(cellCenter, glm::vec2(cx, cy));
                        if (dist < clearance) {
                            blocked[c][r] = true;
                            break;
                        }
                    }
                }
            }
        }
    }
}

glm::vec2 NavGrid::cellToWorld(int col, int row, const glm::vec2& arenaPos) const {
    float x = arenaPos.x + (static_cast<float>(col) * cellSize) + (cellSize * 0.5f);
    float y = arenaPos.y + (static_cast<float>(row) * cellSize) + (cellSize * 0.5f);
    return glm::vec2(x, y);
}

bool NavGrid::worldToCell(const glm::vec2& worldPos, const glm::vec2& arenaPos, int& outCol, int& outRow) const {
    outCol = static_cast<int>(std::floor((worldPos.x - arenaPos.x) / cellSize));
    outRow = static_cast<int>(std::floor((worldPos.y - arenaPos.y) / cellSize));

    if (outCol == cols) outCol = cols - 1;
    if (outRow == rows) outRow = rows - 1;

    return (outCol >= 0 && outCol < cols && outRow >= 0 && outRow < rows);
}

bool NavGrid::isBlocked(int col, int row) const {
    if (col < 0 || col >= cols || row < 0 || row >= rows) return true;
    return blocked[col][row];
}