#pragma once

#include "../config.h"
#include "../view/Mesh.h"

// Режими відтворення геометрії
enum class DrawMode {
    Outline,       // Тільки контур
    Fill,          // Тільки заливка
    FillAndOutline // Заливка + контур
};

// Структура стану візуалу (Матеріал)
struct RenderStyle {
    DrawMode mode          = DrawMode::Outline;
    glm::vec4 fillColor    = glm::vec4(1.0f); // Колір заливки
    glm::vec4 outlineColor = glm::vec4(1.0f); // Колір контуру
    float lineWidth        = 1.0f;            // Товщина ліній контуру
};

class Entity {
protected:
    Mesh* mesh;

public:
    glm::vec2 entityPos;
    RenderStyle style; // Вся графічна конфігурація зберігається тут
    
    Entity(glm::vec2 pos = glm::vec2(0.0f, 0.0f)) 
        : entityPos(pos), mesh(nullptr) {};
    
    virtual ~Entity() {};
    virtual void update(float dt) = 0;

    // Робота з мешем об'єкта
    void setMesh(Mesh* m) { mesh = m; }
    Mesh* getMesh() const { return mesh; }
};