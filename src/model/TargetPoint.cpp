#include "TargetPoint.h"
#include "../view/Renderer.h"

void TargetPoint::drawVisitor(Renderer* renderer) {
    renderer->drawTargetPoint(this);
}