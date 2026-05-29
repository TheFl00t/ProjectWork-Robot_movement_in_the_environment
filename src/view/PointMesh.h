#pragma once

#include "Mesh.h"

class PointMesh : public Mesh {
private:
    void init();

public:
    PointMesh();
    ~PointMesh();

    void draw(GLenum topology) override;
};