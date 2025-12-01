#pragma once

#include "../view/Renderer.h"
#include "../model/Robot.h"
#include "../model/Environment.h"
#include "../model/Point.h"
#include "../model/CircleObstacle.h"
#include "../model/RectObstacle.h"
#include "../model/Line.h"

class Scene {
private:
    Robot* robot;
    Environment* environment;
    Point* collisionPoint; // Точка для дебагу колізії
    Line* velocityLine;    // Ввектор швидкості для дебагу

public:
    bool showCollisionPoint = true; 
    bool showVelocityVector = false;

    Scene(Robot* r, Environment* env);
    ~Scene();

    void update(float dt);           
    void render(Renderer* renderer); 

    Robot* getRobot() { return robot; }

    // Перевірка колізії для гіпотетичної нової позиції
    bool checkCollision(const glm::vec2& newPos);
    
    // Розрахунок точки зіткнення
    glm::vec2 getCollisionPoint(Robot* robot);
    Point* getDebugPoint() const { return collisionPoint; }
    Environment* getEnvironmentPointer() { return environment; }
};