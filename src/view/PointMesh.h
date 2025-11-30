#pragma once

#include "Mesh.h"

class PointMesh : public Mesh {
private:
    unsigned int vertex_count;
    glm::vec4 color;

    void init();
    void updateVertices();

public:
    PointMesh();
    PointMesh(glm::vec4 col);
    ~PointMesh();

    unsigned getVertexCount() const { return vertex_count; }
    void setColor(glm::vec4 col);
    
    void draw() override;
};