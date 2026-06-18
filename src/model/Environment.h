#pragma once

#include "Obstacle.h"
#include "Robot.h"

class Environment : public Entity {
private:
    std::vector<std::unique_ptr<Obstacle>> obstacles;

public:
    float width;
    float height;

    Environment(glm::vec2 pos, float width, float height);
    ~Environment();

    const std::vector<std::unique_ptr<Obstacle>>& getObstacles() const { return obstacles; };
    void addObstacle(std::unique_ptr<Obstacle> obs);
    
    void update(float dt) override;
    bool containsPoint(glm::vec2 point) override;

    bool getBounds(glm::vec2& outMin, glm::vec2& outMax) const override;
    void resizeByGizmo(const glm::vec2& mousePos) override;
    void drawVisitor(class Renderer* renderer) override;

    void setDimensions(float w, float h);

    float intersectRay(const glm::vec2& rayStart, const glm::vec2& rayDir) const;
    
    // Обновленный метод детекции коллизий
    CollisionInfo checkCollisionResult(Robot* robot);
    void removeObstacle(Obstacle* obs);
};