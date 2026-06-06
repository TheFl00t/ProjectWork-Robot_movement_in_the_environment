#include "Point.h"
#include "../view/Renderer.h"

Point::Point(glm::vec2 pos, glm::vec4 col)
    : Entity(pos)
{
    style.mode = DrawMode::Outline;
    style.outlineColor = col;
}

Point::~Point() {}

void Point::update(float dt) {}

void Point::drawVisitor(Renderer* renderer) {
    renderer->drawPoint(this);
}

void Point::setAlpha(float alpha) {
    style.outlineColor.a = alpha;
}