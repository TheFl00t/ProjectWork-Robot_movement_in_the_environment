#include "Point.h"

Point::Point(glm::vec2 pos, glm::vec4 col)
    : Entity(pos)
{
    mesh = new PointMesh();

    // Налаштування матеріалу для дебаг-точки
    style.mode = DrawMode::Outline;
    style.outlineColor = col;
}

Point::~Point() {
    delete mesh;
}

void Point::update(float dt) {}

void Point::setAlpha(float alpha) {
    style.outlineColor.a = alpha;
}