#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "AppState.h"
#include "Scene.h"
#include "MapEditor.h"
#include "../view/GuiManager.h"

/// @brief Головний клас додатку, що керує життєвим циклом програми (ініціалізація, цикл оновлення, рендеринг)
class App {
private:
    GLFWwindow* window = nullptr;           // Вказівник на вікно GLFW
    Scene* scene = nullptr;                 // Сцена
    MapEditor* mapEditor = nullptr;         // Редактор карти
    GuiManager* guiManager = nullptr;       // Менеджер графічного інтерфейсу (ImGui)
    AppState state = AppState::Simulation;  // Поточний стан (Симуляція або Редактор)

    int winWidth = 1024;                    // Ширина вікна
    int winHeight = 768;                    // Висота вікна

    bool imguiInitialized = false;

    float computeDeltaTime();
    void processInput();
    glm::vec2 getScreenToWorldMousePos();

public:
    App() = default;
    ~App();
    void run();
};