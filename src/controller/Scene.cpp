#include "Scene.h"

// Конструктор: ініціалізуємо дебаг-об'єкти з початковими параметрами
Scene::Scene(Robot* r, Environment* env)
    : robot(r), environment(env)
{
    collisionPoint = new Point(glm::vec2(0.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 0.f)); // початкова альфа = 0.0f
    velocityVector = new Line();
}

Scene::~Scene() {
    delete robot;
    delete environment;
    delete collisionPoint;
    delete velocityVector;
}

CollisionInfo Scene::checkHypotheticalCollision(const glm::vec2& pos) {
    glm::vec2 oldPos = robot->entityPos;
    robot->entityPos = pos;
    CollisionInfo info = environment->checkCollisionResult(robot);
    robot->entityPos = oldPos;
    return info;
}

void Scene::update(float dt) {
    if (robot && robot->direction != glm::vec2(0.f)) {
        glm::vec2 totalMove = robot->direction * robot->getVelocity() * dt;
        float moveDist = glm::length(totalMove);

        int subSteps = std::max(1, static_cast<int>(std::ceil(moveDist / (robot->getRadius() * 0.5f))));
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
    } else if (collisionPoint) {
        collisionPoint->setAlpha(0.0f);
    }
    
    if (showVelocityVector && robot) {
        glm::vec2 start = robot->entityPos;
        
        static glm::vec2 lastValidDir = glm::vec2(1.0f, 0.0f); 
        if (robot->direction != glm::vec2(0.f)) {
            lastValidDir = glm::normalize(robot->direction);
            velocityVector->style.outlineColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        } else {
            velocityVector->style.outlineColor = glm::vec4(0.0f, 0.0f, 0.3f, 1.0f);
        }

        float scale;
        if (robot->getVelocity() <= .01f) {
            scale = robot->getRadius();
            velocityVector->style.outlineColor = glm::vec4(0.5f, 0.5f, 1.0f, 1.0f);
        } else {
            scale = (robot->getVelocity() * 0.5f);
        }

        glm::vec2 end = start + (lastValidDir * scale);
        velocityVector->setPoints(start, end);
    }
}

void Scene::render(Renderer* renderer, AppState state) {
    renderer->setShader(ShaderManager::getInstance()->getShader("default"));

    renderer->renderEntity(environment);
    for (auto* obs : environment->getObstacles()) {
        renderer->renderEntity(obs);
    }
    renderer->renderEntity(robot);

    if (state == AppState::Simulation) {
        if (showVelocityVector) {
            renderer->renderEntity(velocityVector);
        }
        if (showCollisionPoint && collisionPoint) {
            renderer->renderEntity(collisionPoint);
        }
    }
}