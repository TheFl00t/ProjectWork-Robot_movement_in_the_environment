#pragma once

#include "Obstacle.h"
#include "Robot.h"
#include "../view/RectMesh.h"

class Environment : public Entity {
private:
    std::vector<Obstacle*> obstacles;
    RectMesh* rectMesh = nullptr;

public:
    float width;
    float height;

    Environment(glm::vec2 pos, float width, float height);
    ~Environment();

    const std::vector<Obstacle*>& getObstacles() const { return obstacles; };
    void addObstacle(Obstacle* obs);
    void update(float dt) override;
    bool containsPoint(glm::vec2 point) override;

    void setDimensions(float w, float h);
    
    // Обновленный метод детекции коллизий
    CollisionInfo checkCollisionResult(Robot* robot);
    void removeObstacle(Obstacle* obs);
};