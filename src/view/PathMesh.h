#pragma once

#include "Mesh.h"
#include <vector>

class PathMesh : public Mesh {
private:
    unsigned int vertexCount = 0;
    void init();

public:
    PathMesh();
    ~PathMesh();

    void updatePath(const std::vector<glm::vec2>& points);
    void draw(GLenum topology) override;
};
