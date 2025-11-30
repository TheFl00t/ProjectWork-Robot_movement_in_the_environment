#pragma once

#include "Obstacle.h"
#include "Robot.h"
#include "../view/RectMesh.h"

class Environment : public Entity {
private:
    float width;
    float height;

    std::vector<Obstacle*> obstacles;

public:
    Environment(glm::vec2 pos, float width, float height);
    ~Environment();

    const std::vector<Obstacle*>& getObstacles() const { return obstacles; };

    void addObstacle(Obstacle* obs);

    void update(float dt) override;
    
    // Перевірка на вихід за межі стін + зіткнення з об'єктами
    bool checkCollision(Robot* robot); 

    // Знаходження точки контакту зі стіною
    glm::vec2 getCollisionPoint(Robot* robot);
};