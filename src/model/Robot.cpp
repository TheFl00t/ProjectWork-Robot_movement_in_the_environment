#include "Robot.h"

Robot::Robot(glm::vec2 pos, float radius, float velocity)
    : Entity(pos),
      radius(radius),
      velocity(velocity),
      direction(glm::vec2(0.0f, 0.0f)),
      startPos(pos),
      startRadius(radius),
      startVelocity(velocity)
{
    mesh = new CircleMesh(radius);
}

Robot::~Robot() {
    delete mesh;
}

void Robot::update(float dt) {
    // ~
}