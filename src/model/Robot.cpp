#include "Robot.h"

Robot::Robot(glm::vec2 pos, float radius, float velocity)
    : Entity(pos), radius(radius), velocity(velocity), direction(glm::vec2(0.f)),
      startPos(pos), startRadius(radius), startVelocity(velocity)
{
    mesh = new CircleMesh(radius);

    style.mode = DrawMode::FillAndOutline;
    style.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.3f);     // Зелений 30% альфа
    style.outlineColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);  // Насичений зелений
    style.lineWidth = 2.5f;
}

Robot::~Robot() {
    delete mesh;
}

void Robot::update(float dt) {
    // Рух контролюється класом Scene
}