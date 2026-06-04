#pragma once

#include "Entity.h"
#include "../view/CircleMesh.h" 
#include "../view/RectMesh.h"

class Robot;

struct CollisionInfo {
    bool collided = false;
    glm::vec2 normal = glm::vec2(0.0f);       // Напрямок від перешкоди до робота
    glm::vec2 contactPoint = glm::vec2(0.0f); // Точка дотику на поверхні
    float depth = 0.0f;                       // Наскільки глибоко робот зайшов у перешкоду
};

class Obstacle : public Entity {
public:
    Obstacle(glm::vec2 pos) : Entity(pos) {};
    virtual ~Obstacle() {};

    virtual CollisionInfo checkCollisionResult(Robot* robot) = 0;
    virtual void updateMesh() = 0;

protected:
    // Допоміжні методи ініціалізації
    void initCircle(float radius, unsigned vertexCount = 360);
    void initRect(float width, float height);
};
