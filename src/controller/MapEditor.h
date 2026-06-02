#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Scene.h"

class MapEditor {
private:
    Obstacle* selectedObstacle = nullptr;

public:
    // Конфігурація для створення нових об'єктів
    int selectedObstacleType = 0; // 0 - Круг, 1 - Прямокутник
    float newCircleRadius = 40.0f;
    float newRectWidth = 80.0f;
    float newRectHeight = 50.0f;
    int newDrawMode = 2; // 0 - Outline, 1 - Fill, 2 - FillAndOutline
    float newFillColor[4] = { 0.5f, 0.5f, 0.5f, 0.4f };
    float newOutlineColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float newLineWidth = 1.5f;

    MapEditor() = default;
    ~MapEditor() = default;

    void update(GLFWwindow* window, Scene* scene, glm::vec2 worldMousePos);
    
    Obstacle* getSelectedObstacle() const { return selectedObstacle; }
    void setSelectedObstacle(Obstacle* obs) { selectedObstacle = obs; }
};