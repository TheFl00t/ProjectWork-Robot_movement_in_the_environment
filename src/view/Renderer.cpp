#include "Renderer.h"
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

Renderer::Renderer() { shaderManager = ShaderManager::getInstance(); }

Renderer::~Renderer() {
    for (auto& [entity, mesh] : viewMeshCache) { delete mesh; }
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
    for (auto& [entity, mesh] : viewMeshCache) {
        delete mesh;
    }
    viewMeshCache.clear();
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