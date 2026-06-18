#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "../controller/AppState.h"
#include "../controller/Scene.h"
#include "../controller/MapEditor.h"

namespace UiTheme {
    // ===== COlORS =====
    inline const ImVec4 ColorLight     = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);     // #999999
    inline const ImVec4 ColorNormal    = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);     // #8080ff
    inline const ImVec4 ColorAccent    = ImVec4(0.502f, 0.753f, 1.0f, 1.0f); // #80c0ff
    inline const ImVec4 ColorImportant = ImVec4(1.0f, 1.0f, 0.502f, 1.0f);   // #ffff80
    inline const ImVec4 ColorCorrect   = ImVec4(0.322f, 1.0f, 0.373f, 1.0f); // #52ff5f
    inline const ImVec4 ColorCritical  = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);     // #ff6666

    // ===== SPACING =====
    // Vectical
    inline const ImVec2 VSpacingSmall  = ImVec2(0.0f, 2.0f);
    inline const ImVec2 VSpacingMedium = ImVec2(0.0f, 5.0f);
    inline const ImVec2 VSpacingLarge  = ImVec2(0.0f, 20.0f);


    // ===== SEPARATOR SETTING =====
    inline const float SeparatorSpacing = 3.0f;
};

namespace ImGui {
    inline void Separator(float spacing_y) {
        ImGui::Dummy(ImVec2(0.0f, spacing_y));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0.0f, spacing_y));
    }
};

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