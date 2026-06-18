#pragma once

#include "AppState.h"
#include "../view/Renderer.h"
#include "../model/Robot.h"
#include "../model/Environment.h"
#include "../model/Point.h"
#include "../model/CircleObstacle.h"
#include "../model/RectObstacle.h"
#include "../model/Line.h"
#include "../model/PathEntity.h"
#include "../model/NavGrid.h"
#include "../model/TargetPoint.h"

class Scene {
private:
    std::unique_ptr<Robot> robot;
    std::unique_ptr<Environment> environment;
    std::unique_ptr<Point> collisionPoint;
    std::unique_ptr<Line> velocityVector;

    bool lastAutoMode = false;
    NavGrid navGrid;
    bool pathNeedsUpdate = false;

public:
    std::unique_ptr<PathEntity> pathEntity;
    std::unique_ptr<TargetPoint> targetPoint;

    bool showGrid = false;
    bool showBlockedCells = false;
    bool isPaused = false;
    bool useLidarForPathfinding = false;
    bool requestGridClear = false;
    float pathfindingCellSize = 20.0f;
    int selectedAlgorithm = 0; // 0 = A*
    int selectedCostmapSource = 0; // 0 = Global Static Environment, 1 = Live Lidar Scan


    bool showCollisionPoint = true; 
    bool showVelocityVector = false;
    bool autonomousMode = false;

    Scene(std::unique_ptr<Robot> r, std::unique_ptr<Environment> env);
    ~Scene();

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    void update(float dt);    
    void updateEditor();       
    void render(Renderer* renderer, AppState state);

    Robot* getRobot() { return robot.get(); }
    Environment* getEnvironmentPointer() { return environment.get(); }
    const std::vector<glm::vec2>& getWaypoints() const { return pathEntity->waypoints; }
    const NavGrid& getNavGrid() const { return navGrid; }

    CollisionInfo checkHypotheticalCollision(const glm::vec2& pos);
    Point* getDebugPoint() const { return collisionPoint.get(); }
    bool checkCollision(const glm::vec2& newPos) { return checkHypotheticalCollision(newPos).collided; }

    void requestPathUpdate() { pathNeedsUpdate = true; }
    void setTarget(const glm::vec2& pos);
};