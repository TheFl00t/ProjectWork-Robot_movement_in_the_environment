#include "Point.h"

Point::Point(glm::vec2 pos, glm::vec4 col)
    : Entity(pos), color(col)
{
    mesh = new PointMesh(color);
}

Point::~Point() {
    delete mesh;
}

void Point::update(float dt) {}

void Point::setAlpha(float alpha) {
    this->color.a = alpha;
    if (auto pMesh = dynamic_cast<PointMesh*>(mesh)) {
        pMesh->setColor(this->color);
    }
}