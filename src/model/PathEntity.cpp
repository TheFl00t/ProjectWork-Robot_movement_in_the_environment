#include "PathEntity.h"
#include "../view/Renderer.h"

void PathEntity::drawVisitor(Renderer* renderer) {
    renderer->drawPath(this);
}