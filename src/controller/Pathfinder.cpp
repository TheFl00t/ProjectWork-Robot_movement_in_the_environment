#include "Pathfinder.h"
#include <vector>
#include <limits>
#include <algorithm>

float Pathfinder::heuristic(int c1, int r1, int c2, int r2) {
    return std::sqrt(static_cast<float>((c1 - c2) * (c1 - c2) + (r1 - r2) * (r1 - r2)));
}

std::vector<glm::vec2> Pathfinder::findPath(const NavGrid& grid, const glm::vec2& startPos, const glm::vec2& targetPos, const glm::vec2& arenaPos) {
    std::vector<glm::vec2> worldPath;

    int startCol, startRow, targetCol, targetRow;
    if (!grid.worldToCell(startPos, arenaPos, startCol, startRow) ||
        !grid.worldToCell(targetPos, arenaPos, targetCol, targetRow)) {
        return worldPath; // Точки поза межами сітки
    }

    // Якщо клітинка цілі заблокована, шукаємо найближчу вільну поруч
    if (grid.isBlocked(targetCol, targetRow)) {
        int bestCol = targetCol;
        int bestRow = targetRow;
        bool foundFreeCell = false;
        
        int maxRadius = std::max(grid.cols, grid.rows);
        
        // Шукаємо пошарово (радіус 1, 2, 3...)
        for (int r_search = 1; r_search < maxRadius && !foundFreeCell; ++r_search) {
            std::vector<std::pair<int, int>> layerCandidates;
            
            for (int dc = -r_search; dc <= r_search; ++dc) {
                for (int dr = -r_search; dr <= r_search; ++dr) {
                    // перевіряємо лише периметр поточного шару-квадрата
                    if (std::abs(dc) != r_search && std::abs(dr) != r_search) continue;
                    
                    int tc = targetCol + dc;
                    int tr = targetRow + dr;
                    
                    if (tc >= 0 && tc < grid.cols && tr >= 0 && tr < grid.rows && !grid.isBlocked(tc, tr)) {
                        layerCandidates.push_back({tc, tr});
                    }
                }
            }
            
            // Якщо в поточному радіусі знайшли вільні клітини
            if (!layerCandidates.empty()) {
                float minDistToRobot = std::numeric_limits<float>::infinity();
                
                // Знаходимо серед них ту, яка ближче до фізичного положення робота
                for (const auto& candidate : layerCandidates) {
                    glm::vec2 candWorld = grid.cellToWorld(candidate.first, candidate.second, arenaPos);
                    float dist = glm::distance(startPos, candWorld);
                    
                    if (dist < minDistToRobot) {
                        minDistToRobot = dist;
                        bestCol = candidate.first;
                        bestRow = candidate.second;
                    }
                }
                foundFreeCell = true;
            }
        }
        
        if (!foundFreeCell) return worldPath; 
        
        targetCol = bestCol;
        targetRow = bestRow;
    }

    std::priority_queue<GridNode, std::vector<GridNode>, std::greater<GridNode>> openSet;
    
    int totalCells = grid.cols * grid.rows;
    std::vector<float> gScore(totalCells, std::numeric_limits<float>::infinity());
    std::vector<int> cameFrom(totalCells, -1);

    int startIdx = startCol + startRow * grid.cols;
    gScore[startIdx] = 0.0f;
    openSet.push({startCol, startRow, 0.0f, heuristic(startCol, startRow, targetCol, targetRow)});

    int dCol[] = {0, 0, -1, 1, -1, 1, -1, 1};
    int dRow[] = {-1, 1, 0, 0, -1, -1, 1, 1};
    float edgeCosts[] = {1.0f, 1.0f, 1.0f, 1.0f, 1.414f, 1.414f, 1.414f, 1.414f};

    bool found = false;

    while (!openSet.empty()) {
        GridNode current = openSet.top();
        openSet.pop();
        
        int currIdx = current.col + current.row * grid.cols;
        if (current.gScore > gScore[currIdx] + 1e-4f)
            continue;

        if (current.col == targetCol && current.row == targetRow) {
            found = true;
            break;
        }

        for (int i = 0; i < 8; ++i) {
            int neighborCol = current.col + dCol[i];
            int neighborRow = current.row + dRow[i];

            if (grid.isBlocked(neighborCol, neighborRow)) continue;

            // Заборона зрізання кутів при діагональному русі
            if (i >= 4) {
                if (grid.isBlocked(current.col + dCol[i], current.row) ||
                    grid.isBlocked(current.col, current.row + dRow[i])) {
                    continue; 
                }
            }

            float tentativeGScore = gScore[currIdx] + edgeCosts[i];
            int neighborIdx = neighborCol + neighborRow * grid.cols;

            if (tentativeGScore < gScore[neighborIdx]) {
                cameFrom[neighborIdx] = currIdx;
                gScore[neighborIdx] = tentativeGScore;
                float f = tentativeGScore + heuristic(neighborCol, neighborRow, targetCol, targetRow);
                openSet.push({neighborCol, neighborRow, tentativeGScore, f});
            }
        }
    }

    if (found) {
        // Реконструкція шляху через індекси плоских векторів
        int currNode = targetCol + targetRow * grid.cols;
        int startNode = startCol + startRow * grid.cols;

        while (currNode != startNode) {
            int c = currNode % grid.cols;
            int r = currNode / grid.cols;
            worldPath.push_back(grid.cellToWorld(c, r, arenaPos));
            currNode = cameFrom[currNode];
        }
        std::reverse(worldPath.begin(), worldPath.end());
    }

    return worldPath;
}