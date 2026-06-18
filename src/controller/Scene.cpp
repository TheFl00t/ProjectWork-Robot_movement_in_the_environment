#include "Scene.h"
#include "LidarProcessor.h"
#include "Pathfinder.h"

// Конструктор: ініціалізуємо дебаг-об'єкти з початковими параметрами
Scene::Scene(std::unique_ptr<Robot> r, std::unique_ptr<Environment> env)
    : robot(std::move(r)), environment(std::move(env))
{
    collisionPoint = std::make_unique<Point>(glm::vec2(0.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 0.f));
    velocityVector = std::make_unique<Line>();
    pathEntity = std::make_unique<PathEntity>();
    targetPoint = std::make_unique<TargetPoint>();
}

Scene::~Scene() {}

CollisionInfo Scene::checkHypotheticalCollision(const glm::vec2& pos) {
    glm::vec2 oldPos = robot->entityPos;
    robot->entityPos = pos;
    CollisionInfo info = environment->checkCollisionResult(robot.get());
    robot->entityPos = oldPos;
    return info;
}

void Scene::update(float dt) {
    if (!robot || !environment || !targetPoint) return;

    pathEntity->elementSize = pathfindingCellSize;

    pathEntity->arenaPos = environment->entityPos;
    pathEntity->arenaWidth = environment->width;
    pathEntity->arenaHeight = environment->height;

    // 1. ОЧИСТКА / ПЕРЕРАХУНОК СІТКИ (Оптимізовано: не запікаємо всю сітку при зміні цілі в GSE)
    bool shouldBakeGrid = false;

    if (navGrid.cols == 0 || requestGridClear) {
        shouldBakeGrid = true;
    } else if (useLidarForPathfinding && targetPoint->active && targetPoint->isMovingTo) {
        shouldBakeGrid = true; // Лідар оновлює сітку динамічно в русі
    }

    if (shouldBakeGrid) {
        navGrid.cellSize = pathfindingCellSize;
        navGrid.bake(*environment, robot->getRadius(), robot.get(), useLidarForPathfinding, requestGridClear);
        requestGridClear = false; 
    }

    // 2. ПОШУК ШЛЯХУ А* (Викликається лише при зміні цілі АБО при оновленні сітки лідара)
    if (pathNeedsUpdate || (shouldBakeGrid && useLidarForPathfinding)) {
        if (autonomousMode && targetPoint->active) {
            pathEntity->waypoints = Pathfinder::findPath(navGrid, robot->entityPos, targetPoint->entityPos, environment->entityPos);
        }
        pathNeedsUpdate = false; // Скидаємо тригер пошуку шляху
    }

    // ЛОГІКА АВТОПІЛОТА
    if (autonomousMode && targetPoint->active && targetPoint->isMovingTo) {
        if (!pathEntity->waypoints.empty()) {
            glm::vec2 nextPoint = pathEntity->waypoints[0];
            glm::vec2 toPoint = nextPoint - robot->entityPos;
            float distToPoint = glm::length(toPoint);
            float arrivalTolerance = 4.0f;

            if (distToPoint > arrivalTolerance) {
                robot->direction = glm::normalize(toPoint);
            } else {
                pathEntity->waypoints.erase(pathEntity->waypoints.begin());
                if (pathEntity->waypoints.empty()) {
                    robot->direction = glm::vec2(0.0f);
                    targetPoint->active = false;
                    targetPoint->isMovingTo = false;
                } else {
                    glm::vec2 toNext = pathEntity->waypoints[0] - robot->entityPos;
                    if (glm::length(toNext) > 1e-5f)
                        robot->direction = glm::normalize(toNext);
                }
            }
        } else {
            robot->direction = glm::vec2(0.0f);
            targetPoint->isMovingTo = false;
        }
    } else if (autonomousMode && !targetPoint->isMovingTo) {
        robot->direction = glm::vec2(0.0f);
    }

    if (autonomousMode != lastAutoMode) {
        if (!autonomousMode) {
            pathEntity->waypoints.clear();
            isPaused = false;
        } else {
            if (robot) robot->direction = glm::vec2(0.0f);
            if (targetPoint->active) pathNeedsUpdate = true;
        }
        lastAutoMode = autonomousMode;
    }

    if (isPaused) {
        if (robot && environment) {
            LidarProcessor::updateLidar(*robot, *environment);
        }
        return;
    }

    // ФІЗИКА РУХУ ТА ОБРОБКИ КОЛІЗІЙ
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

    if (robot && environment) {
        LidarProcessor::updateLidar(*robot, *environment);
    }
}

void Scene::render(Renderer* renderer, AppState state) {
    renderer->setShader(ShaderManager::getInstance()->getShader("default"));

    renderer->renderEntity(environment.get());
    for (const auto& obs : environment->getObstacles()) {
        renderer->renderEntity(obs.get());
    }

    // Відображення ліній сітки та заблокованих ячеєк
    if (showGrid || showBlockedCells) {
        renderer->drawGrid(this);
    }

    if (state == AppState::Simulation) {
        renderer->drawLidar(robot.get());
        if (autonomousMode && targetPoint->active) {
            renderer->renderEntity(pathEntity.get());
        }
    } 

    // Рендеринг інтерактивної цілі у будь-якому стані програми
    if (targetPoint->active) {
        renderer->renderEntity(targetPoint.get());
    }

    renderer->renderEntity(robot.get());

    if (state == AppState::Simulation) {
        if (showVelocityVector) {
            renderer->renderEntity(velocityVector.get());
        }
        if (showCollisionPoint && collisionPoint) {
            renderer->renderEntity(collisionPoint.get());
        }
    }
}

void Scene::updateEditor() {
    if (!robot || !environment) return;

    pathEntity->arenaPos = environment->entityPos;
    pathEntity->arenaWidth = environment->width;
    pathEntity->arenaHeight = environment->height;

    if (navGrid.cols == 0 || requestGridClear) {
        navGrid.cellSize = pathfindingCellSize;
        navGrid.bake(*environment, robot->getRadius(), robot.get(), false, requestGridClear);
        requestGridClear = false;
    }
}

void Scene::setTarget(const glm::vec2& pos) {
    targetPoint->entityPos = pos;
    targetPoint->active = true;
    targetPoint->isMovingTo = false;
    pathNeedsUpdate = true;
}