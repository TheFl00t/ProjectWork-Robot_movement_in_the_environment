#pragma once

#include "Mesh.h"

class LineMesh : public Mesh {
private:
    unsigned int vertex_count = 2;
    void init();

public:
    LineMesh();
    ~LineMesh();

    void updateLine(glm::vec2 start, glm::vec2 end);
    
    void draw() override;
};