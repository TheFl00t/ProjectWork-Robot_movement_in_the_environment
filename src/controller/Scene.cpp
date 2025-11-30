#include "Scene.h"

Scene::Scene(Robot* r, Environment* env)
    : robot(r), environment(env)
{
    collisionPoint = new Point(glm::vec2(0.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 0.f)); // default альфа = 0
    velocityLine = new Line();
}

Scene::~Scene() {
    delete robot;
    delete environment;
    delete collisionPoint;
    delete velocityLine;
}

bool Scene::checkCollision(const glm::vec2& newPos) {
    glm::vec2 oldPos = robot->entityPos;
    robot->entityPos = newPos;

    bool collision = environment->checkCollision(robot);

    robot->entityPos = oldPos;
    return collision;
}

glm::vec2 Scene::getCollisionPoint(Robot* robot) {
    // 1. Перевірка стін
    glm::vec2 wallPoint = environment->getCollisionPoint(robot);
    if (wallPoint != robot->entityPos) {
        return wallPoint;
    }

    // 2. Перевірка перешкод
    for (auto* obs : environment->getObstacles()) {
        if (obs->checkCollision(robot)) {
            if (auto circle = dynamic_cast<CircleObstacle*>(obs)) {
                return circle->getCollisionPoint(robot);
            }
            if (auto rect = dynamic_cast<RectObstacle*>(obs)) {
                return rect->getCollisionPoint(robot);
            }
        }
    }
    return robot->entityPos;
}

void Scene::update(float dt) {
    // Розрахунок нової позиції
    glm::vec2 newPos = robot->entityPos + robot->direction * robot->velocity * dt;

    if (!checkCollision(newPos)) {
        robot->entityPos = newPos;
        collisionPoint->setAlpha(0.0f); // Ховаємо точку
    } 
    else {
        glm::vec2 safePos = robot->entityPos;
        robot->entityPos = newPos;

        glm::vec2 contact = getCollisionPoint(robot);

        robot->entityPos = safePos;

        collisionPoint->entityPos = contact;
        collisionPoint->setAlpha(1.0f); // Показуємо точку
    }
    
    // Оновлення візуального вектора швидкості
    if (showVelocityVector) {
        glm::vec2 start = robot->entityPos;
        float scale = (robot->velocity < 1.0f) ? robot->radius : (robot->velocity * 0.5f);
        glm::vec2 end = start + (robot->direction * scale);
        velocityLine->setPoints(start, end);
    }
}

void Scene::render(Renderer* renderer) {
    // Малюємо середовище
    renderer->setShader(ShaderManager::getInstance()->getShader("obstacle"));
    renderer->renderEnvironment(environment);
    
    // Малюємо стіни
    renderer->setShader(ShaderManager::getInstance()->getShader("walls"));
    renderer->renderWalls(environment);

    // Малюємо робота
    renderer->setShader(ShaderManager::getInstance()->getShader("robot"));
    renderer->renderRobot(robot);

    // Малюємо вектор швидкості
    if (showVelocityVector) {
        renderer->setShader(ShaderManager::getInstance()->getShader("walls"));
        renderer->renderLine(velocityLine);
    }

    // Малюємо точку колізії
    if (showCollisionPoint && collisionPoint) {
        renderer->setShader(ShaderManager::getInstance()->getShader("point"));
        renderer->renderPoint(collisionPoint);
    }
}