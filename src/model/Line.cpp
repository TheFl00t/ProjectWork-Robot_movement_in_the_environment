#include "Line.h"

Line::Line() : Entity(glm::vec2(0, 0)) {
    mesh = new LineMesh();
}

Line::~Line() {
    delete mesh;
}

void Line::setPoints(glm::vec2 start, glm::vec2 end) {
    if (auto lineMesh = dynamic_cast<LineMesh*>(mesh)) {
        lineMesh->updateLine(start, end);
    }
}