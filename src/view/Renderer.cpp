#include "Renderer.h"

#include "../controller/Scene.h"

#include "../model/Entity.h"
#include "../model/Robot.h"
#include "../model/Environment.h"
#include "../model/CircleObstacle.h"
#include "../model/RectObstacle.h"
#include "../model/Line.h"
#include "../model/Point.h"

#include "CircleMesh.h"
#include "RectMesh.h"
#include "LineMesh.h"
#include "PointMesh.h"
#include "PathMesh.h"

Renderer::Renderer() { shaderManager = ShaderManager::getInstance(); }

Renderer::~Renderer() {
    for (auto& [entity, mesh] : viewMeshCache) { delete mesh; }
    for (auto& [robot, mesh] : lidarMeshCache) { delete mesh; }
    for (auto& [scene, mesh] : pathMeshCache) { delete mesh; }
}

Renderer* Renderer::getInstance() {
    static Renderer instance;
    return &instance;
}

void Renderer::setShader(Shader* shader) { currentShader = shader; }
void Renderer::setProjection(const glm::mat4& proj) { projection = proj; }

void Renderer::applyProjectionToAllShaders() {
    for (auto& [name, shader] : shaderManager->getAllShaders()) {
        shader->use();
        shader->setMat4("uProjection", projection);
    }
}

void Renderer::clearCache() {
    for (auto& [entity, mesh] : viewMeshCache) { delete mesh; }
    viewMeshCache.clear();

    for (auto& [robot, mesh] : lidarMeshCache) { delete mesh; }
    lidarMeshCache.clear();

    for (auto& [scene, mesh] : pathMeshCache) { delete mesh; }
    pathMeshCache.clear();
}

void Renderer::freeEntityMesh(const Entity* entity) {
    auto it = viewMeshCache.find(entity);
    if (it != viewMeshCache.end()) {
        delete it->second;
        viewMeshCache.erase(it);
    }
}

void Renderer::renderEntity(Entity* entity) {
    if (!entity || !currentShader) return;
    entity->drawVisitor(this);
}

void Renderer::internalRender(const Entity* entity, Mesh* mesh) {
    currentShader->use();
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(entity->entityPos, 0.0f));
    currentShader->setMat4("uModel", model);
    
    switch (entity->style.mode) {
        case DrawMode::Outline:
            glLineWidth(entity->style.lineWidth);
            currentShader->setVec4("uColor", entity->style.outlineColor);
            mesh->draw(GL_LINE_LOOP);
            glLineWidth(1.0f);
            break;
        case DrawMode::Fill:
            currentShader->setVec4("uColor", entity->style.fillColor);
            mesh->draw(GL_TRIANGLE_FAN);
            break;
        case DrawMode::FillAndOutline:
            currentShader->setVec4("uColor", entity->style.fillColor);
            mesh->draw(GL_TRIANGLE_FAN);
            glLineWidth(entity->style.lineWidth);
            currentShader->setVec4("uColor", entity->style.outlineColor);
            mesh->draw(GL_LINE_LOOP);
            glLineWidth(1.0f);
            break;
    }
}

void Renderer::drawRobot(const Robot* robot) {
    Mesh*& mesh = viewMeshCache[robot];
    if (!mesh) mesh = new CircleMesh(robot->getRadius());
    
    static_cast<CircleMesh*>(mesh)->setRadius(robot->getRadius());
    internalRender(robot, mesh);
}

void Renderer::drawEnvironment(const Environment* env) {
    Mesh*& mesh = viewMeshCache[env];
    if (!mesh) mesh = new RectMesh(env->width, env->height, false);
    
    static_cast<RectMesh*>(mesh)->updateDimensions(env->width, env->height);
    internalRender(env, mesh);
}

void Renderer::drawCircleObstacle(const CircleObstacle* circle) {
    Mesh*& mesh = viewMeshCache[circle];
    if (!mesh) mesh = new CircleMesh(circle->getRadius());
    
    static_cast<CircleMesh*>(mesh)->setRadius(circle->getRadius());
    internalRender(circle, mesh);
}

void Renderer::drawRectObstacle(const RectObstacle* rect) {
    Mesh*& mesh = viewMeshCache[rect];
    if (!mesh) mesh = new RectMesh(rect->getWidth(), rect->getHeight(), true);
    
    static_cast<RectMesh*>(mesh)->updateDimensions(rect->getWidth(), rect->getHeight());
    internalRender(rect, mesh);
}

void Renderer::drawLine(const Line* line) {
    Mesh*& mesh = viewMeshCache[line];
    if (!mesh) mesh = new LineMesh();
    
    static_cast<LineMesh*>(mesh)->updateLine(line->ptStart, line->ptEnd);

    currentShader->use();
    currentShader->setMat4("uModel", glm::mat4(1.0f)); 
    currentShader->setVec4("uColor", line->style.outlineColor);
    mesh->draw(GL_LINES);
}

void Renderer::drawPoint(const Point* point) {
    Mesh*& mesh = viewMeshCache[point];
    if (!mesh) mesh = new PointMesh();
    internalRender(point, mesh);
}

void Renderer::drawLidar(const Robot* robot) {
    if (!robot || !currentShader) return;
    if (!robot->getLidar().visible) return; // перевірка видимості

    LidarMesh*& mesh = lidarMeshCache[robot];
    if (!mesh) mesh = new LidarMesh();

    mesh->updateLines(robot->entityPos, robot->getLidar().getImpactPoints());

    currentShader->use();
    currentShader->setMat4("uModel", glm::mat4(1.0f));
    currentShader->setVec4("uColor", robot->getLidar().rayColor);
    mesh->draw(GL_LINES);
}

void Renderer::drawPath(const PathEntity* path) {
    if (!path || !currentShader || path->waypoints.empty()) return;

    currentShader->use();
    currentShader->setVec4("uColor", path->style.outlineColor); 

    if (path->renderType == 0) {
        // === ЛІНІЯ ===
        PathMesh*& mesh = pathMeshCache[path]; 
        if (!mesh) mesh = new PathMesh();

        mesh->updatePath(path->waypoints);
        currentShader->setMat4("uModel", glm::mat4(1.0f)); 
        
        glLineWidth(path->style.lineWidth);
        mesh->draw(GL_LINE_STRIP);
        glLineWidth(1.0f);
    } 
    else if (path->renderType == 1) {
        // === ТОЧКИ ===
        static CircleMesh pathDotMesh(4.0f, 16);
        pathDotMesh.setRadius(4.0f);

        for (const auto& pt : path->waypoints) {
            // Якщо центр вейпоінту через математику округлення виліз за стіну арени — не малюємо його
            if (pt.x < path->arenaPos.x || pt.x > path->arenaPos.x + path->arenaWidth ||
                pt.y < path->arenaPos.y || pt.y > path->arenaPos.y + path->arenaHeight) {
                continue;
            }
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pt, 0.0f));
            currentShader->setMat4("uModel", model);
            pathDotMesh.draw(GL_TRIANGLE_FAN);
        }
    } 
    else if (path->renderType == 2) {
        // === КВАДРАТИ ===
        // використовуємо false (нецентрований меш) для точного розрахунку зрізів
        static RectMesh pathSquareMesh(path->elementSize, path->elementSize, false);

        for (const auto& pt : path->waypoints) {
            // Обчислюємо початковий лівий верхній кут квадрата ячейки
            float x = pt.x - path->elementSize * 0.5f;
            float y = pt.y - path->elementSize * 0.5f;

            float w = path->elementSize;
            float h = path->elementSize;

            // Кліппінг по правій та нижній межах
            if (x + w > path->arenaPos.x + path->arenaWidth) {
                w = (path->arenaPos.x + path->arenaWidth) - x;
            }
            if (y + h > path->arenaPos.y + path->arenaHeight) {
                h = (path->arenaPos.y + path->arenaHeight) - y;
            }

            // Кліппінг по лівій та верхній межах
            if (x < path->arenaPos.x) {
                w -= (path->arenaPos.x - x);
                x = path->arenaPos.x;
            }
            if (y < path->arenaPos.y) {
                h -= (path->arenaPos.y - y);
                y = path->arenaPos.y;
            }

            // Якщо ячейка повністю опинилась за межами через зріз стіни
            if (w <= 0.0f || h <= 0.0f) continue;

            // Звужуємо меш квадрата під фактичний залишок простору
            pathSquareMesh.updateDimensions(w, h);

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
            currentShader->setMat4("uModel", model);
            pathSquareMesh.draw(GL_TRIANGLE_FAN);
        }
        // Повертаємо дефолтний стабільний розмір для наступних ітерацій
        pathSquareMesh.updateDimensions(path->elementSize, path->elementSize);
    }
}

void Renderer::drawGrid(const Scene* scene) {
    if (!scene || !currentShader) return;

    const NavGrid& grid = scene->getNavGrid();
    Environment* env = const_cast<Scene*>(scene)->getEnvironmentPointer();
    if (!env) return;

    glm::vec2 arenaLeftTop = env->entityPos;

    // 1. ВІДМАЛЬОВУВАННЯ ЗАПОВНЕННЯ ЗАБЛОКОВАНИХ ЯЧЕЄК
    if (scene->showBlockedCells) {
        static RectMesh cellMesh(grid.cellSize, grid.cellSize, false);
        cellMesh.updateDimensions(grid.cellSize, grid.cellSize);

        currentShader->use();
        currentShader->setVec4("uColor", glm::vec4(1.0f, 0.0f, 0.0f, 0.18f));

        for (int c = 0; c < grid.cols; ++c) {
            for (int r = 0; r < grid.rows; ++r) {
                if (grid.isBlocked(c, r)) {
                    float x = arenaLeftTop.x + static_cast<float>(c) * grid.cellSize;
                    float y = arenaLeftTop.y + static_cast<float>(r) * grid.cellSize;

                    float cellW = grid.cellSize;
                    float cellH = grid.cellSize;

                    // Обрізаємо ячейку, якщо її край вилазить за праву або нижню межу арени
                    if (x + cellW > arenaLeftTop.x + env->width) {
                        cellW = (arenaLeftTop.x + env->width) - x;
                    }
                    if (y + cellH > arenaLeftTop.y + env->height) {
                        cellH = (arenaLeftTop.y + env->height) - y;
                    }

                    // Якщо ячейка повністю за межами через зміну розмірів, не малюємо
                    if (cellW <= 0.0f || cellH <= 0.0f) continue;

                    // Тимчасово змінюємо розмір мешу для цієї конкретної крайньої ячейки
                    cellMesh.updateDimensions(cellW, cellH);

                    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
                    currentShader->setMat4("uModel", model);
                    cellMesh.draw(GL_TRIANGLE_FAN);
                }
            }
        }
        // Відновлюємо дефолтний стабільний розмір ячейки для наступних кадрів
        cellMesh.updateDimensions(grid.cellSize, grid.cellSize);
    }

    // 2. ВІДМАЛЬОВУВАННЯ ЛІНІЙ СІТКИ
    if (scene->showGrid) {
        static LineMesh gridLineMesh;
        currentShader->use();
        currentShader->setMat4("uModel", glm::mat4(1.0f));
        currentShader->setVec4("uColor", glm::vec4(1.0f, 1.0f, 1.0f, 0.08f)); // Дуже тонкі білі лінії
        glLineWidth(1.0f);

        // Вертикальні лінії сітки
        for (int c = 0; c <= grid.cols; ++c) {
            float x = arenaLeftTop.x + static_cast<float>(c) * grid.cellSize;
            
            // Якщо лінія виходить за межі правої стіни
            if (x > arenaLeftTop.x + env->width) {
                x = arenaLeftTop.x + env->width;
                gridLineMesh.updateLine(glm::vec2(x, arenaLeftTop.y), glm::vec2(x, arenaLeftTop.y + env->height));
                gridLineMesh.draw(GL_LINES);
                break; // Усі наступні лінії гарантовано далі, виходимо з циклу
            }
            
            gridLineMesh.updateLine(glm::vec2(x, arenaLeftTop.y), glm::vec2(x, arenaLeftTop.y + env->height));
            gridLineMesh.draw(GL_LINES);
        }
        
        // Горизонтальні лінії сітки
        for (int r = 0; r <= grid.rows; ++r) {
            float y = arenaLeftTop.y + static_cast<float>(r) * grid.cellSize;
            
            // Якщо лінія виходить за межі нижньої стіни
            if (y > arenaLeftTop.y + env->height) {
                y = arenaLeftTop.y + env->height;
                gridLineMesh.updateLine(glm::vec2(arenaLeftTop.x, y), glm::vec2(arenaLeftTop.x + env->width, y));
                gridLineMesh.draw(GL_LINES);
                break; // Усі наступні лінії гарантовано далі, виходимо з циклу
            }
            
            gridLineMesh.updateLine(glm::vec2(arenaLeftTop.x, y), glm::vec2(arenaLeftTop.x + env->width, y));
            gridLineMesh.draw(GL_LINES);
        }
    }
}

void Renderer::drawTargetPoint(const TargetPoint* target) {
    if (!target || target->style.outlineColor.a <= 0.0f) return;

    Mesh*& mesh = viewMeshCache[target];
    if (!mesh) mesh = new LineMesh();

    currentShader->use();
    currentShader->setMat4("uModel", glm::mat4(1.0f));
    
    float s = 10.0f;
    glm::vec2 p = target->entityPos;


    // обводка
    glLineWidth(target->style.lineWidth + 2.0f);
    currentShader->setVec4("uColor", glm::vec4(0.0f, 0.0f, 0.0f, target->style.outlineColor.a));

    // Перша діагональ обводки
    static_cast<LineMesh*>(mesh)->updateLine(glm::vec2(p.x - s, p.y - s), glm::vec2(p.x + s, p.y + s));
    mesh->draw(GL_LINES);

    // Друга діагональ обводки
    static_cast<LineMesh*>(mesh)->updateLine(glm::vec2(p.x - s, p.y + s), glm::vec2(p.x + s, p.y - s));
    mesh->draw(GL_LINES);

    // основний хрест
    glLineWidth(target->style.lineWidth);
    currentShader->setVec4("uColor", target->style.outlineColor);

    // Перша робоча діагональ
    static_cast<LineMesh*>(mesh)->updateLine(glm::vec2(p.x - s, p.y - s), glm::vec2(p.x + s, p.y + s));
    mesh->draw(GL_LINES);

    // Друга робоча діагональ
    static_cast<LineMesh*>(mesh)->updateLine(glm::vec2(p.x - s, p.y + s), glm::vec2(p.x + s, p.y - s));
    mesh->draw(GL_LINES);

    glLineWidth(1.0f);
}