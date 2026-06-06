#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Scene.h"
#include "../model/Entity.h"

enum class EditorTool {
    Select,
    Create
};

class MapEditor {
private:
    Entity* selectedEntity = nullptr;
    glm::vec2 dragOffset = glm::vec2(0.0f);
    bool isDragging = false;
    bool wasLeftPressed = false;

public:
    EditorTool currentTool = EditorTool::Select;
    int spawnObjectType = 0;

    // Конфігурація для створення нових об'єктів
    float newCircleRadius = 40.0f;
    float newRectWidth = 80.0f;
    float newRectHeight = 50.0f;
    int newDrawMode = 2; 
    float newFillColor[4] = { 0.5f, 0.5f, 0.5f, 0.4f };
    float newOutlineColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float newLineWidth = 1.5f;

    MapEditor() = default;
    ~MapEditor() = default;

    void update(GLFWwindow* window, Scene* scene, glm::vec2 worldMousePos);
    
    Entity* getSelectedEntity() const { return selectedEntity; }
    void setSelectedEntity(Entity* ent) { selectedEntity = ent; }
};