#pragma once

#include <GLFW/glfw3.h>
#include "../controller/AppState.h"
#include "../controller/Scene.h"
#include "../controller/MapEditor.h"

class GuiManager {
public:
    GuiManager() = default;
    ~GuiManager() = default;

    // Отрисовка всего интерфейса в зависимости от текущего состояния
    void render(GLFWwindow* window, Scene* scene, MapEditor* editor, AppState& currentState, int winWidth, int winHeight);
};