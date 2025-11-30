#pragma once

#include "../config.h"
#include "../view/Mesh.h"

class Entity {
protected:
    Mesh* mesh;

public:
    glm::vec2 entityPos;
    
    Entity(glm::vec2 pos = glm::vec2(0.0f, 0.0f)) 
        : entityPos(pos), mesh(nullptr) {};
    
    virtual ~Entity() {};

    virtual void update(float dt) = 0;

    // Робота з мешем
    void setMesh(Mesh* m) { mesh = m; }
    Mesh* getMesh() const { return mesh; }
};