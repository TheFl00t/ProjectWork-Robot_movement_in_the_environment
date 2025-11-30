#include "Obstacle.h"

void Obstacle::initCircle(float radius, unsigned vertexCount) {
    this->mesh = new CircleMesh(radius, vertexCount);
}

void Obstacle::initRect(float width, float height) {
    this->mesh = new RectMesh(width, height);
}