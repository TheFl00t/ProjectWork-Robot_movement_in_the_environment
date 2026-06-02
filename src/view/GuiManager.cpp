#include "GuiManager.h"
#include "imgui.h"
#include "../controller/ConfigLoader.h"
#include "../view/Renderer.h"
#include <iostream>
#include <algorithm>

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

void GuiManager::render(GLFWwindow* window, Scene*& scene, MapEditor* editor, AppState& currentState, int& winWidth, int& winHeight) {
    float windowW = 300.0f;
    float padding = 20.0f;
    float posX = ImGui::GetIO().DisplaySize.x - windowW - padding;
    float posY = padding;

    ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(windowW, 520), ImGuiCond_Appearing);

    if (resetGuiPos) {
        ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Always);
        resetGuiPos = false; 
    }

    ImGui::Begin("Simulation Control", NULL, ImGuiWindowFlags_NoResize);

    // 1. Інформація
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "[ Info ]");
    ImGui::Text("Performance: %.1f FPS", ImGui::GetIO().Framerate);
    
    Robot* robot = scene ? scene->getRobot() : nullptr;
    if (robot) {
        ImGui::Text("Robot Pos: (%.1f, %.1f)", robot->entityPos.x, robot->entityPos.y);
        Point* p = scene->getDebugPoint();
        ImGui::Text("CollisionPoint Pos: (%.1f, %.1f)", p->entityPos.x, p->entityPos.y);
    }
    ImGui::Separator();

    // 2. Фізика (працює лише в режимі симуляції)
    if (robot) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "[ Physics ]");
        ImGui::BeginDisabled(currentState == AppState::Editor);
        ImGui::SliderFloat("Velocity", &robot->velocity, 0.0f, 600.0f);
        if (ImGui::SliderFloat("Radius", &robot->radius, 5.0f, 100.0f)) {
            if (auto mesh = dynamic_cast<CircleMesh*>(robot->getMesh())) {
                mesh->setRadius(robot->radius);
            }
        }
        ImGui::EndDisabled();
    }
    ImGui::Separator();

    // 3. Налагодження
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[ Debug ]");
    ImGui::Checkbox("Show Collision Point", &scene->showCollisionPoint);
    ImGui::Checkbox("Show Velocity Vector", &scene->showVelocityVector);
    ImGui::Separator();

    // 4. Редактор карт
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[ Map Editor ]");
    bool isEditMode = (currentState == AppState::Editor);
    if (ImGui::Checkbox("Enable Edit Mode", &isEditMode)) {
        currentState = isEditMode ? AppState::Editor : AppState::Simulation;
        if (currentState == AppState::Editor && robot) {
            robot->direction = glm::vec2(0.f); // Зупиняємо рух
        }
    }

    if (currentState == AppState::Editor && editor) {
        ImGui::Indent();
        ImGui::RadioButton("Circle", &editor->selectedObstacleType, 0); ImGui::SameLine();
        ImGui::RadioButton("Rectangle", &editor->selectedObstacleType, 1);

        if (editor->selectedObstacleType == 0) {
            ImGui::SliderFloat("Radius##New", &editor->newCircleRadius, 10.0f, 150.0f);
        } else {
            ImGui::SliderFloat("Width##New", &editor->newRectWidth, 10.0f, 300.0f);
            ImGui::SliderFloat("Height##New", &editor->newRectHeight, 10.0f, 300.0f);
        }

        ImGui::Combo("Draw Mode##New", &editor->newDrawMode, "Outline\0Fill\0FillAndOutline\0");
        ImGui::ColorEdit4("Fill Color##New", editor->newFillColor);
        ImGui::ColorEdit4("Outline Color##New", editor->newOutlineColor);
        ImGui::SliderFloat("Line Width##New", &editor->newLineWidth, 1.0f, 5.0f);
        ImGui::Unindent();
    }
    ImGui::Separator();

    // 5. Система
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "[ System ]");
    if (ImGui::Button("Reset Position", ImVec2(150, 25))) {
        if (robot) {
            robot->entityPos = robot->startPos;
            robot->direction = glm::vec2(0.f);
            scene->checkCollision(robot->entityPos);
        }
    }
    
    std::string configPath = ConfigLoader::getConfigPath("config.json", false);

    if (ImGui::Button("Open Config File", ImVec2(150, 25))) {   
        ShellExecuteA(NULL, "open", "notepad.exe", configPath.c_str(), NULL, SW_SHOW);
    }
    
    if (ImGui::Button("Save Map", ImVec2(150, 25))) {
        ConfigLoader::saveScene(configPath, scene, winWidth, winHeight);
    }

    if (ImGui::Button("Reload Config", ImVec2(150, 25))) {
        int newW, newH;
        ConfigLoader::loadWindowSize(configPath, newW, newH);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        newW = std::clamp(newW, 800, mode->width);
        newH = std::clamp(newH, 600, mode->height);

        if (newW != winWidth || newH != winHeight) {
            winWidth = newW;
            winHeight = newH;
            glfwSetWindowSize(window, winWidth, winHeight);
            glViewport(0, 0, winWidth, winHeight);

            glm::mat4 newProj = glm::ortho(0.0f, (float)winWidth, (float)winHeight, 0.0f, -1.0f, 1.0f);
            Renderer::getInstance()->setProjection(newProj);
            Renderer::getInstance()->applyProjectionToAllShaders();
        }

        Scene* newScene = ConfigLoader::loadScene(configPath, winWidth, winHeight);
        if (newScene != nullptr) {
            delete scene;
            scene = newScene;
        }
    }

    ImGui::End();
}