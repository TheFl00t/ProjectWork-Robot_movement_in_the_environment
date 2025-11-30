#include "Renderer.h"

Renderer::Renderer() {
    shaderManager = ShaderManager::getInstance();
}

Renderer* Renderer::getInstance() {
    static Renderer instance;
    return &instance;
}

void Renderer::setShader(Shader* shader) {
    currentShader = shader;
}

void Renderer::setProjection(const glm::mat4& proj) {
    projection = proj;
}

void Renderer::applyProjectionToAllShaders() {
    for (auto& [name, shader] : shaderManager->getAllShaders()) {
        shader->use();
        shader->setMat4("uProjection", projection);
    }
}

void Renderer::render(Robot* robot, Environment* env) {
    renderEnvironment(env);
    renderRobot(robot);
}

void Renderer::renderEnvironment(Environment* env) {
    for (Obstacle* obs : env->getObstacles()) {
        renderObstacle(obs);
    }
}

void Renderer::renderWalls(Environment* env) {
    currentShader->use();
    if (auto mesh = env->getMesh()) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(env->entityPos, 0.0f));
        currentShader->setMat4("uModel", model);
        mesh->draw();
    }
}

void Renderer::renderRobot(Robot* robot) {
    currentShader->use();
    if (auto mesh = robot->getMesh()) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(robot->entityPos, 0.0f));
        currentShader->setMat4("uModel", model);
        mesh->draw();
    }
}

void Renderer::renderObstacle(Obstacle* obstacle) {
    currentShader->use();
    if (auto mesh = obstacle->getMesh()) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(obstacle->entityPos, 0.0f));
        currentShader->setMat4("uModel", model);
        mesh->draw();
    }
}

void Renderer::renderPoint(Point* point) {
    currentShader->use();
    if (auto mesh = point->getMesh()) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(point->entityPos, 0.0f));
        currentShader->setMat4("uModel", model);
        mesh->draw();
    }
}

void Renderer::renderLine(Line* line) {
    currentShader->use();
    // Для лінії модельна матриця одинична, бо координати вершин 
    // вже є глобальними (мировими), ми їх обчислюємо в Scene::update
    if (auto mesh = line->getMesh()) {
        currentShader->setMat4("uModel", glm::mat4(1.0f));
        mesh->draw();
    }
}