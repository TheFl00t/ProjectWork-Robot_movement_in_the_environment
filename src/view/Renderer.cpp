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

void Renderer::renderEntity(Entity* entity) {
    if (!entity || !currentShader) return;

    currentShader->use();
    if (auto mesh = entity->getMesh()) {
        // Позиціонування об'єкта у світових координатах
        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(entity->entityPos, 0.0f));
        currentShader->setMat4("uModel", model);
        
        // Рендеринг на основі структури станів (Матеріалу)
        switch (entity->style.mode) {
            case DrawMode::Outline:
                glLineWidth(entity->style.lineWidth);
                currentShader->setVec4("uColor", entity->style.outlineColor);
                mesh->draw(GL_LINE_LOOP);
                glLineWidth(1.0f); // Скидання на стандартні значення
                break;

            case DrawMode::Fill:
                currentShader->setVec4("uColor", entity->style.fillColor);
                mesh->draw(GL_TRIANGLE_FAN);
                break;

            case DrawMode::FillAndOutline:
                // Прохід 1: Заливка тіла
                currentShader->setVec4("uColor", entity->style.fillColor);
                mesh->draw(GL_TRIANGLE_FAN);
                
                // Прохід 2: Відтворення контуру поверх тіла
                glLineWidth(entity->style.lineWidth);
                currentShader->setVec4("uColor", entity->style.outlineColor);
                mesh->draw(GL_LINE_LOOP);
                glLineWidth(1.0f); // Скидання на стандартні значення
                break;
        }
    }
}