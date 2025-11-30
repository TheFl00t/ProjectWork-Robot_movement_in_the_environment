#include "Environment.h"

Environment::Environment(glm::vec2 pos, float width, float height) :
    Entity(pos), width(width), height(height) 
{
    setMesh(new RectMesh(width, height));
}

Environment::~Environment() {
    for (auto obs : obstacles) {
        delete obs;
    }
    delete mesh;
}

void Environment::addObstacle(Obstacle* obs) {
    obstacles.push_back(obs);
}

void Environment::update(float dt) {
    // ~
}

bool Environment::checkCollision(Robot* robot) {
    // 1. Перевірка меж (Стіни)
    float left   = entityPos.x;
    float right  = entityPos.x + width;
    float top    = entityPos.y;
    float bottom = entityPos.y + height;

    if (robot->entityPos.x - robot->radius < left || 
        robot->entityPos.x + robot->radius > right ||
        robot->entityPos.y - robot->radius < top  || 
        robot->entityPos.y + robot->radius > bottom)
    {
        return true;
    }

    // 2. Перевірка внутрішніх перешкод
    for (auto* obs : obstacles) {
        if (obs->checkCollision(robot))
            return true;
    }
    return false;
}

glm::vec2 Environment::getCollisionPoint(Robot* robot) {
    float left   = entityPos.x;
    float right  = entityPos.x + width;
    float top    = entityPos.y;
    float bottom = entityPos.y + height;

    float r = robot->radius;
    float x = robot->entityPos.x;
    float y = robot->entityPos.y;

    // Логіка визначення, яку стіну ми зачепили
    if (x - r < left)
        return glm::vec2(left, y);
    else if (x + r > right)
        return glm::vec2(right, y);
    else if (y - r < top)
        return glm::vec2(x, top);
    else if (y + r > bottom)
        return glm::vec2(x, bottom);

    return robot->entityPos;
}