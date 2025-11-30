#pragma once

#include "Mesh.h"

class RectMesh : public Mesh {
private:
    unsigned int vertex_count;
    float width;
    float height;
    
    void init();

public:
    RectMesh();
    RectMesh(float width, float height);
    ~RectMesh();

    unsigned getVertexCount() const { return vertex_count; }

    void draw() override;
};