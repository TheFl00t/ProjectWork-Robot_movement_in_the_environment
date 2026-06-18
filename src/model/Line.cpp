#include "Line.h"
#include "../view/Renderer.h"

Line::Line() 
    : Entity(glm::vec2(0.0f, 0.0f))
{
    style.mode = DrawMode::Outline;
    style.outlineColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    style.lineWidth = 2.0f;
}

Line::~Line() {}

void Line::drawVisitor(Renderer* renderer) {
    renderer->drawLine(this);
}

void Line::setPoints(glm::vec2 start, glm::vec2 end) {
    ptStart = start;
    ptEnd = end;
}