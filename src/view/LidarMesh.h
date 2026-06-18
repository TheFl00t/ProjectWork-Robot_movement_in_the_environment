#pragma once

#include "Mesh.h"
#include <vector>

class LidarMesh : public Mesh {
private:
    unsigned int vertexCount = 0;
    void init();

public:
    LidarMesh();
    ~LidarMesh();

    void updateLines(const glm::vec2& start, const std::vector<glm::vec2>& impacts);
    void draw(GLenum topology) override;
};