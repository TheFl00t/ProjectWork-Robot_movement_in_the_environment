#pragma once

#include <GLFW/glfw3.h>
#include "Scene.h"

class MapEditor {
private:
    Obstacle* selectedObstacle = nullptr; // Указатель на выбранный объект

public:
    MapEditor() = default;
    ~MapEditor() = default;

    void update(GLFWwindow* window, Scene* scene, glm::vec2 worldMousePos);
    
    Obstacle* getSelectedObstacle() const { return selectedObstacle; }
    void setSelectedObstacle(Obstacle* obs) { selectedObstacle = obs; }
};