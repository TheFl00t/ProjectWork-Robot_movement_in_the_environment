#pragma once

#include "Mesh.h"

class RectMesh : public Mesh {
private:
    unsigned int vertex_count;
    float width;
    float height;
    bool centered;

    void init();

public:
    RectMesh();
    RectMesh(float width, float height, bool centered = true);
    ~RectMesh();

    unsigned getVertexCount() const { return vertex_count; }
    void draw(GLenum topology) override;

    void updateDimensions(float newWidth, float newHeight);
};