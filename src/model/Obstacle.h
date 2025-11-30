#pragma once

#include "Entity.h"
#include "../view/CircleMesh.h" 
#include "../view/RectMesh.h"

class Robot;

class Obstacle : public Entity {
public:
    Obstacle(glm::vec2 pos) : Entity(pos) {};
    virtual ~Obstacle() {};

    virtual bool checkCollision(Robot* robot) = 0;
    virtual void updateMesh() = 0;

protected:
    // Допоміжні методи ініціалізації
    void initCircle(float radius, unsigned vertexCount = 360);
    void initRect(float width, float height);
};