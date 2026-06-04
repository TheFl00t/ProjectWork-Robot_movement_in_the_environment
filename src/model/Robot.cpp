    #include "Robot.h"

    Robot::Robot(glm::vec2 pos, float radius, float velocity)
        : Entity(pos), radius(radius), velocity(velocity), direction(glm::vec2(0.f)),
        startPos(pos), startRadius(radius), startVelocity(velocity)
    {
        circleMesh = new CircleMesh(radius);
        mesh = circleMesh;

        style.mode = DrawMode::FillAndOutline;
        style.fillColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.3f);
        style.outlineColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
        style.lineWidth = 2.5f;
    }

    Robot::~Robot() {
        delete mesh;
    }

    void Robot::update(float dt) {
        // Рух контролюється класом Scene
    }

    bool Robot::containsPoint(glm::vec2 point) {
        glm::vec2 dir = point - entityPos;
        return glm::dot(dir, dir) <= (radius * radius);
    }

    void Robot::setRadius(float newRadius) {
        radius = std::clamp(newRadius, 0.0f, 1000.0f);
        if (circleMesh) {
            circleMesh->setRadius(radius);
        }
    }

    void Robot::setVelocity(float newVelocity) {
        velocity = std::clamp(newVelocity, 0.0f, 10000.0f);
    }
