#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "AppState.h"
#include "Scene.h"
#include "MapEditor.h"
#include "../view/GuiManager.h"

class App {
private:
    GLFWwindow* window = nullptr;
    Scene* scene = nullptr;
    MapEditor* mapEditor = nullptr;
    GuiManager* guiManager = nullptr;
    AppState state = AppState::Simulation;

    int winWidth = 1024;
    int winHeight = 768;

    float computeDeltaTime();
    void processInput();
    glm::vec2 getScreenToWorldMousePos();

public:
    App() = default;
    ~App();
    void run();
};