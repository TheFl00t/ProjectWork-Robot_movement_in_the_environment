#include "Scene.h"

// Конструктор: ініціалізуємо дебаг-об'єкти з початковими параметрами
Scene::Scene(Robot* r, Environment* env)
    : robot(r), environment(env)
{
    collisionPoint = new Point(glm::vec2(0.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 0.f)); // початкова альфа = 0.0f
    velocityLine = new Line();
}

Scene::~Scene() {
    delete robot;
    delete environment;
    delete collisionPoint;
    delete velocityLine;
}

CollisionInfo Scene::checkHypotheticalCollision(const glm::vec2& pos) {
    glm::vec2 oldPos = robot->entityPos;
    robot->entityPos = pos;

    CollisionInfo info = environment->checkCollisionResult(robot);

    robot->entityPos = oldPos;
    return info;
}

void Scene::update(float dt) {
    if (!robot || robot->direction == glm::vec2(0.f)) {
        collisionPoint->setAlpha(0.0f);
        return;
    }

    glm::vec2 totalMove = robot->direction * robot->velocity * dt;
    float moveDist = glm::length(totalMove);

    // Динамічні підкроки (субстепінг) для запобігання проходженню крізь стіни
    int subSteps = std::max(1, static_cast<int>(std::ceil(moveDist / (robot->radius * 0.5f))));
    subSteps = std::min(subSteps, 32);
    glm::vec2 subMove = totalMove / (float)subSteps;
    
    CollisionInfo finalCollision;
    bool hasCollision = false;

    for (int i = 0; i < subSteps; ++i) {
        glm::vec2 nextPos = robot->entityPos + subMove;
        CollisionInfo info = checkHypotheticalCollision(nextPos);
        
        if (!info.collided) {
            robot->entityPos = nextPos;
        } else {
            // Бінарний пошук із достроковим виходом для точного притискання до стіни
            glm::vec2 low = robot->entityPos;
            glm::vec2 high = nextPos;
            
            for (int j = 0; j < 8; ++j) { 
                glm::vec2 diff = high - low;
                if (glm::dot(diff, diff) < 0.01f) break;
                
                glm::vec2 mid = (low + high) * 0.5f;
                CollisionInfo midInfo = checkHypotheticalCollision(mid);
                if (!midInfo.collided) {
                    low = mid;
                } else {
                    high = mid;
                }
            }
            robot->entityPos = low;
            
            // Отримуємо точні геометричні параметри зіткнення для фінальної позиції
            finalCollision = checkHypotheticalCollision(high);
            hasCollision = true;
            break;
        }
    }

    if (!hasCollision) {
        collisionPoint->setAlpha(0.0f);
    } else {
        collisionPoint->entityPos = finalCollision.contactPoint;
        collisionPoint->setAlpha(1.0f);
    }
    
    if (showVelocityVector) {
        glm::vec2 start = robot->entityPos;
        float scale = (robot->velocity < 1.0f) ? robot->radius : (robot->velocity * 0.5f);
        glm::vec2 end = start + (robot->direction * scale);
        velocityLine->setPoints(start, end);
    }
}

void Scene::render(Renderer* renderer) {
    renderer->setShader(ShaderManager::getInstance()->getShader("default"));

    // Малюємо оточення (стіни арени)
    renderer->renderEntity(environment);

    // Малюємо внутрішні перешкоди
    for (auto* obs : environment->getObstacles()) {
        renderer->renderEntity(obs);
    }

    // Малюємо робота
    renderer->renderEntity(robot);

    // Малюємо дебаг-вектор швидкості
    if (showVelocityVector) {
        renderer->renderEntity(velocityLine);
    }

    // Малюємо дебаг-точку зіткнення
    if (showCollisionPoint && collisionPoint) {
        renderer->renderEntity(collisionPoint);
    }
}