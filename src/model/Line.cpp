#include "Line.h"

Line::Line() 
    : Entity(glm::vec2(0.0f, 0.0f))
{
    lineMesh = new LineMesh();
    mesh = lineMesh;

    style.mode = DrawMode::Outline;
    style.outlineColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    style.lineWidth = 2.0f;
}

Line::~Line() {
    delete mesh;
}

void Line::setPoints(glm::vec2 start, glm::vec2 end) {
    if (lineMesh) {
        lineMesh->updateLine(start, end);
    }
}