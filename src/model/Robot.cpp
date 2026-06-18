#include "Robot.h"
#include "../constants.h"
#include "../view/Renderer.h"

Robot::Robot(glm::vec2 pos, float radius, float velocity)
    : Entity(pos), radius(radius), velocity(velocity), direction(glm::vec2(0.f)),
    startPos(pos), startRadius(radius), startVelocity(velocity),
    lidar()
{
    style.mode = DrawMode::FillAndOutline;
    style.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.3f);
    style.outlineColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    style.lineWidth = 2.5f;
}

Robot::~Robot() {}

void Robot::update(float dt) {
    // Рух контролюється класом Scene
}

bool Robot::containsPoint(glm::vec2 point) {
    glm::vec2 dir = point - entityPos;
    return glm::dot(dir, dir) <= (radius * radius);
}

bool Robot::getBounds(glm::vec2& outMin, glm::vec2& outMax) const {
    outMin = entityPos - glm::vec2(radius);
    outMax = entityPos + glm::vec2(radius);
    return true;
}

void Robot::resizeByGizmo(const glm::vec2& mousePos) {
    float calculatedRadius = mousePos.x - entityPos.x;
    setRadius(calculatedRadius);
}

void Robot::drawVisitor(Renderer* renderer) {
    renderer->drawRobot(this);
}

void Robot::setRadius(float newRadius) {
    radius = std::clamp(newRadius, ROBOT_RADIUS_MIN, ROBOT_RADIUS_MAX);
}

void Robot::setVelocity(float newVelocity) {
    velocity = std::clamp(newVelocity, ROBOT_VELOCITY_MIN, ROBOT_VELOCITY_MAX);
}
