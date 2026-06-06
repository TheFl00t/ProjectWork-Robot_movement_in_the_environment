#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../controller/AppState.h"
#include "../controller/Scene.h"
#include "../controller/MapEditor.h"

class GuiManager {
public:
    bool resetGuiPos = false;
    bool resetControlPanel = false;
    bool resetEditorWorkspace = false;
    bool showUi = true;

    GuiManager() = default;
    ~GuiManager() = default;

    void render(GLFWwindow* window, Scene*& scene, MapEditor* editor, AppState& currentState, int& winWidth, int& winHeight);
};