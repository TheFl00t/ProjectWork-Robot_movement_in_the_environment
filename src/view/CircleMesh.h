#pragma once

#include "Mesh.h"

class CircleMesh : public Mesh {
private:
    unsigned int vertex_count;
    float radius;

    void init();
    void updateVertices();

public:
    CircleMesh(float r, unsigned vertCount = 360);
    CircleMesh();
    ~CircleMesh();

    float getRadius() const { return radius; }
    void setRadius(float r);
    
    void draw() override;
};