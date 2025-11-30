#pragma once

#include "Entity.h"
#include "../view/PointMesh.h"

class Point : public Entity {
public:
    glm::vec4 color;

    Point(glm::vec2 pos, glm::vec4 col);
    ~Point();

    void update(float dt) override;
    
    void setAlpha(float alpha);
};