#include "GuiManager.h"
#include "imgui.h"
#include "../controller/ConfigLoader.h"
#include "../view/Renderer.h"
#include "../constants.h"

#include "../model/Robot.h"
#include "../model/Environment.h"
#include "../model/CircleObstacle.h"
#include "../model/RectObstacle.h"

#include <iostream>
#include <algorithm>
#include <filesystem>

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

void GuiManager::render(GLFWwindow* window, Scene*& scene, MapEditor* editor, AppState& currentState, int& winWidth, int& winHeight) {
    float padding = 10.0f;
    float menuBarOffset = 25.0f;
    const float controlAlignX = 115.0f;
    std::string configPath = ConfigLoader::getConfigPath("config.json", false);

    if (!ImGui::GetIO().WantTextInput && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        ConfigLoader::saveScene(configPath, scene, winWidth, winHeight);
    }

    if (resetGuiPos) {
        resetControlPanel = true;
        resetEditorWorkspace = true;
        resetGuiPos = false;
    }

    // Динамічні обмеження області перегляду
    float screenW = static_cast<float>(winWidth);
    float screenH = static_cast<float>(winHeight);
    float maxAvailableHeight = screenH - menuBarOffset - (2 * padding);

    // Таймери та цілі для ефекту червоного спалаху при спробі видалити системний об'єкт
    static float flashTimer = 0.0f;
    static Entity* flashTarget = nullptr;
    if (flashTimer > 0.0f) {
        flashTimer -= ImGui::GetIO().DeltaTime;
    }

    // === MENU BAR ===
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Config File")) {
                ShellExecuteA(NULL, "open", "notepad.exe", configPath.c_str(), NULL, SW_SHOW);
            }
            if (ImGui::MenuItem("Save Active Map", "Ctrl+S")) {
                ConfigLoader::saveScene(configPath, scene, winWidth, winHeight);
            }
            if (ImGui::MenuItem("Reload from Config File")) {
                ConfigLoader::saveAppSettings("settings.json", scene);
                Scene* newScene = ConfigLoader::loadScene(configPath, winWidth, winHeight);
                if (newScene != nullptr) {
                    Renderer::getInstance()->clearCache();
                    delete scene; 
                    scene = newScene; 
                    
                    ConfigLoader::loadAppSettings("settings.json", scene);
                    editor->setSelectedEntity(nullptr);
                }
            }

            ImGui::Separator();

            if (ImGui::MenuItem("Save Current as Default")) {
                // Запікаємо поточні налаштування користувача як нові дефолтні
                ConfigLoader::saveAppSettings("settings_default.json", scene);
            }
            if (ImGui::MenuItem("Reset to Defaults")) {
                // Завантажуємо еталонні налаштування в пам'ять програми
                ConfigLoader::loadAppSettings("settings_default.json", scene);
                // Одразу синхронізуємо файл поточної сесії
                ConfigLoader::saveAppSettings("settings.json", scene);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Reset Camera View", nullptr, false, false)) {
                // TODO: Reset scale and mouse drag offsets later
            }
            if (ImGui::MenuItem("Center View on Robot", nullptr, false, false)) {
                // TODO: Center camera framework on robot position
            }

            ImGui::Separator();

            if (ImGui::BeginMenu("Grid")) {
                if (scene) {
                    ImGui::MenuItem("Show Grid Lines", nullptr, &scene->showGrid);
                    ImGui::MenuItem("Show Blocked Cell Fills", nullptr, &scene->showBlockedCells);
                } else {
                    ImGui::TextDisabled("No active scene");
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Reset Layout")) {
                resetGuiPos = true;
            }
            ImGui::MenuItem("Show Interface", "Tab", &showUi);
            ImGui::EndMenu();
        }

        ImGui::Separator();
        
        ImGui::Spacing(); ImGui::SameLine(0, 15);
        ImGui::Text("App Mode:"); 
        
        ImGui::SameLine(0, 15);
        bool isEditMode = (currentState == AppState::Editor);
        if (ImGui::RadioButton("Simulation", !isEditMode)) {
            currentState = AppState::Simulation;
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Shortcut: 1");
        }
          
        ImGui::SameLine(0, 15);
        if (ImGui::RadioButton("Map Editor", isEditMode)) {
            currentState = AppState::Editor;
            if (scene && scene->getRobot()) scene->getRobot()->direction = glm::vec2(0.f);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Shortcut: 2");
        }

        ImGui::SameLine(0, 40);
        ImGui::TextColored(UiTheme::ColorLight, "|   FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::EndMainMenuBar();
    }

    // === SIMULATION ===
    if (showUi && currentState == AppState::Simulation && scene) {
        float sideWidth = 320.0f;
        Robot* robot = scene->getRobot();

        // ВІКНО А: РОБОТ ТА СЕНСОРИ (Права сторона екрану)
        ImGui::SetNextWindowPos(ImVec2(screenW - sideWidth - padding, menuBarOffset + padding), resetControlPanel ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(sideWidth, 430), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(280, 300), ImVec2(400, maxAvailableHeight));
        
        ImGui::Begin("Control Panel", NULL, ImGuiWindowFlags_None);

        ImVec2 posA = ImGui::GetWindowPos(); 
        ImVec2 sizeA = ImGui::GetWindowSize(); 
        ImVec2 clampedA = posA;
        
        if (clampedA.x < padding) clampedA.x = padding;
        if (clampedA.x + sizeA.x > screenW - padding) clampedA.x = screenW - sizeA.x - padding;
        if (clampedA.y < menuBarOffset + padding) clampedA.y = menuBarOffset + padding;
        if (clampedA.y + sizeA.y > screenH - padding) clampedA.y = screenH - sizeA.y - padding;
        if (clampedA.x != posA.x || clampedA.y != posA.y) ImGui::SetWindowPos(clampedA);

        if (robot) {
            ImGui::TextColored(UiTheme::ColorNormal, "[ Information ]");
            ImGui::Text("Robot Position: (%.1f, %.1f)", robot->entityPos.x, robot->entityPos.y);
            ImGui::Spacing();
            
            ImGui::TextColored(UiTheme::ColorNormal, "[ Movement Physics ]");
            
            ImGui::AlignTextToFramePadding(); ImGui::Text("Velocity:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
            float tempRobotVelocity = robot->getVelocity();
            if (ImGui::SliderFloat("##Velocity", &tempRobotVelocity, ROBOT_VELOCITY_MIN, ROBOT_VELOCITY_MAX, "%.1f")) {
                robot->setVelocity(tempRobotVelocity);
            }
            
            ImGui::AlignTextToFramePadding(); ImGui::Text("Radius:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
            float tempRobotRadius = robot->getRadius();
            if (ImGui::SliderFloat("##Radius", &tempRobotRadius, ROBOT_RADIUS_MIN, ROBOT_RADIUS_MAX, "%.1f")) {
                robot->setRadius(tempRobotRadius);
            }

            ImGui::Separator();
            ImGui::TextColored(UiTheme::ColorNormal, "[ Debug Settings ]");
            ImGui::Checkbox("Collision Point", &scene->showCollisionPoint);
            ImGui::Checkbox("Velocity Vector", &scene->showVelocityVector);
            ImGui::Checkbox("PAUSE Simulation Movement", &scene->isPaused);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Shortcut: Ctrl+Space");

            if (scene->isPaused) {
                ImGui::TextColored(UiTheme::ColorCritical, "Status: SIMULATION PAUSED");
            } else {
                ImGui::TextColored(UiTheme::ColorCorrect, "Status: SIMULATION ACTIVE");
            }

            ImGui::Separator();
            ImGui::TextColored(UiTheme::ColorNormal, "[ Lidar Settings ]");

            Lidar& lidar = robot->getLidar();
            ImGui::Checkbox("Show Lidar", &lidar.visible);

            ImGui::AlignTextToFramePadding(); ImGui::Text("Ray Count:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
            int rayCount = lidar.getRayCount();
            if (ImGui::SliderInt("##Rays", &rayCount, LIDAR_RAY_MIN, LIDAR_RAY_MAX)) {
                lidar.setRayCount(rayCount);
            }

            ImGui::AlignTextToFramePadding(); ImGui::Text("Distance:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
            float maxDist = lidar.getMaxDistance();
            if (ImGui::SliderFloat("##LidarDist", &maxDist, LIDAR_DIST_MIN, LIDAR_DIST_MAX, "%.0f")) {
                lidar.setMaxDistance(maxDist);
            }

            ImGui::AlignTextToFramePadding(); ImGui::Text("FOV:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
            float fovDeg = glm::degrees(lidar.getFov());
            if (ImGui::SliderFloat("##LidarFov", &fovDeg, glm::degrees(LIDAR_FOV_MIN), glm::degrees(LIDAR_FOV_MAX), "%.0f deg")) {
                lidar.setFov(glm::radians(fovDeg));
            }

            ImGui::AlignTextToFramePadding(); ImGui::Text("Ray Color:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
            ImGui::ColorEdit4("##LidarColor", &lidar.rayColor.r);

            ImGui::Separator();
            if (ImGui::Button("Reset Robot Position", ImVec2(-1, 25))) {
                robot->entityPos = robot->startPos;
                robot->direction = glm::vec2(0.f);
                scene->checkCollision(robot->entityPos);
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", "Shortcut: R");
            
            if (ImGui::Button("Reset Robot Velocity", ImVec2(-1, 25))) {
                robot->setVelocity(robot->startVelocity);
                scene->checkCollision(robot->entityPos);
            }
            if (ImGui::Button("Reset Robot Radius", ImVec2(-1, 25))) {
                robot->setRadius(robot->startRadius);
                scene->checkCollision(robot->entityPos);
            }
        }
        ImGui::End();

        // ВІКНО Б: НАВІГАЦІЯ ТА АЛГОРИТМИ (Ліва сторона екрану)
        ImGui::SetNextWindowPos(ImVec2(padding, menuBarOffset + padding), resetControlPanel ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(sideWidth, 390), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(280, 250), ImVec2(400, maxAvailableHeight));
        
        ImGui::Begin("Navigation Workspace", NULL, ImGuiWindowFlags_None);

        ImVec2 posB = ImGui::GetWindowPos(); ImVec2 sizeB = ImGui::GetWindowSize(); ImVec2 clampedB = posB;
        if (clampedB.x < padding) clampedB.x = padding;
        if (clampedB.x + sizeB.x > screenW - padding) clampedB.x = screenW - sizeB.x - padding;
        if (clampedB.y < menuBarOffset + padding) clampedB.y = menuBarOffset + padding;
        if (clampedB.y + sizeB.y > screenH - padding) clampedB.y = screenH - sizeB.y - padding;
        if (clampedB.x != posB.x || clampedB.y != posB.y) ImGui::SetWindowPos(clampedB);

        ImGui::TextColored(UiTheme::ColorNormal, "[ Autopilot Controls ]");
        if (ImGui::Checkbox("Enable Autonomous Mode", &scene->autonomousMode)) {
            if (!scene->autonomousMode && robot) {
                robot->direction = glm::vec2(0.0f);
            }
        }

        if (scene->autonomousMode) {
            ImGui::Separator(UiTheme::SeparatorSpacing);
            ImGui::TextColored(UiTheme::ColorNormal, "[ Pathfinding Algorithm ]");
            const char* algorithms[] = { "A* (8-Way Grid-Based)" };
            ImGui::SetNextItemWidth(-1);
            ImGui::Combo("##AlgorithmSelector", &scene->selectedAlgorithm, algorithms, IM_ARRAYSIZE(algorithms));

            ImGui::Dummy(UiTheme::VSpacingMedium);
            ImGui::TextColored(UiTheme::ColorNormal, "[ Mapping Data Source ]");
            const char* costmap_sources[] = { "Global Static Environment", "Live Lidar Scan" };
            ImGui::SetNextItemWidth(-1);
            if (ImGui::Combo("##CostmapSourceSelector", &scene->selectedCostmapSource, costmap_sources, IM_ARRAYSIZE(costmap_sources))) {
                if (scene->selectedCostmapSource == 1) {
                    scene->useLidarForPathfinding = true;
                } else {
                    scene->useLidarForPathfinding = false;
                }
                scene->requestGridClear = true;
                scene->requestPathUpdate(); 
            }

            if (scene->useLidarForPathfinding) {
                ImGui::Dummy(UiTheme::VSpacingSmall);
                if (ImGui::Button("Reset Lidar Map", ImVec2(-1, 25))) {
                    scene->requestGridClear = true;
                    scene->requestPathUpdate();
                }
            }

            ImGui::Dummy(UiTheme::VSpacingMedium);
            ImGui::TextColored(UiTheme::ColorImportant, "* Right-Click or Drag Target Cross to re-position.");

            if (scene->targetPoint->active) {
                ImGui::Text("Target Node: (%.1f, %.1f)", scene->targetPoint->entityPos.x, scene->targetPoint->entityPos.y);
            } else {
                ImGui::Text("Status: Waiting for Target point...");
            }
            ImGui::Separator(UiTheme::SeparatorSpacing);

            ImGui::BeginDisabled(!scene->targetPoint->active);
            if (scene->targetPoint->isMovingTo) {
                ImVec4 buttonColor = UiTheme::ColorCritical;
                buttonColor.w = 0.4f; // Встановлюємо прозорість для злиття з фоном вікна
                
                ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
                if (ImGui::Button("STOP ROBOT MOVEMENT", ImVec2(-1, 30))) {
                    scene->targetPoint->isMovingTo = false;
                }
                ImGui::PopStyleColor();
            } else {
                ImVec4 buttonColor = UiTheme::ColorCorrect;
                buttonColor.w = 0.4f; // Встановлюємо прозорість для злиття з фоном вікна
                
                ImGui::PushStyleColor(ImGuiCol_Button, buttonColor);
                if (ImGui::Button("START ROBOT MOVEMENT", ImVec2(-1, 30))) {
                    scene->targetPoint->isMovingTo = true;
                    scene->requestPathUpdate();
                }
                ImGui::PopStyleColor();
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Shortcut: Space");
            ImGui::EndDisabled();
        }

        ImGui::Separator(UiTheme::SeparatorSpacing);
        ImGui::TextColored(UiTheme::ColorNormal, "[ Grid & Map Configuration ]");
        ImGui::Checkbox("Show Grid Lines", &scene->showGrid);
        ImGui::Checkbox("Show Blocked Cell Fills", &scene->showBlockedCells);
        
        ImGui::AlignTextToFramePadding(); ImGui::Text("Cell Size:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
        if (ImGui::SliderFloat("##CellSizeSlider", &scene->pathfindingCellSize, 8.0f, 40.0f, "%.0f px")) {
            scene->requestGridClear = true;
            scene->requestPathUpdate();
        }

        // Налаштування візуалу шляху
        ImGui::Separator(UiTheme::SeparatorSpacing);
        ImGui::TextColored(UiTheme::ColorNormal, "[ Path Visual Settings ]");

        ImGui::AlignTextToFramePadding(); ImGui::Text("Path Color:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
        ImGui::ColorEdit4("##PathColor", &scene->pathEntity->style.outlineColor.r);

        ImGui::AlignTextToFramePadding(); ImGui::Text("Path Style:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
        ImGui::Combo("##PathRenderType", &scene->pathEntity->renderType, "Line\0Dots\0Squares\0");
        
        if (scene->pathEntity->renderType == 0) {
            ImGui::AlignTextToFramePadding(); ImGui::Text("Line Width:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
            ImGui::SliderFloat("##PathLineWidth", &scene->pathEntity->style.lineWidth, 1.0f, 5.0f, "%.1f");
        }

        // Налаштування візуалу цілі
        ImGui::Separator(UiTheme::SeparatorSpacing);
        ImGui::TextColored(UiTheme::ColorNormal, "[ Target Visual Settings ]");

        ImGui::AlignTextToFramePadding(); ImGui::Text("Cross Color:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
        ImGui::ColorEdit4("##TargetCrossColor", &scene->targetPoint->style.outlineColor.r);

        ImGui::AlignTextToFramePadding(); ImGui::Text("Line Width:"); ImGui::SameLine(100); ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##TargetCrossLineWidth", &scene->targetPoint->style.lineWidth, 1.0f, 5.0f, "%.1f");

        ImGui::End();
        if (resetControlPanel) resetControlPanel = false;
    }

    // === MAP EDITOR ===
    if (currentState == AppState::Editor && editor && scene) {
        Environment* env = scene->getEnvironmentPointer();
        Robot* robot = scene->getRobot();
        const auto& obstacles = env->getObstacles();
        Entity* currentSelected = editor->getSelectedEntity();

        // ОБРОБКА ГАРЯЧОЇ КЛАВІШІ CTRL+D (DESELECT)
        if (!ImGui::GetIO().WantTextInput && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D, false) && currentSelected) {
            editor->setSelectedEntity(nullptr);
            currentSelected = nullptr;
        }

        // ОБРОБКА ГАРЯЧОЇ КЛАВІШІ DELETE
        if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Delete) && currentSelected) {
            if (currentSelected == robot || currentSelected == env) {
                flashTimer = 0.4f; // Вмикаємо червоний спалах на 400мс
                flashTarget = currentSelected;
            } else {
                Renderer::getInstance()->freeEntityMesh(currentSelected);
                env->removeObstacle(static_cast<Obstacle*>(currentSelected));
                editor->setSelectedEntity(nullptr);
                currentSelected = nullptr;
                scene->requestGridClear = true;
            }
        }

        // ОБРОБКА ГАРЯЧИХ КЛАВІШ [S, C]
        if (!ImGui::GetIO().WantTextInput) {
            if (ImGui::IsKeyPressed(ImGuiKey_S)) {
                editor->currentTool = EditorTool::Select;
            } else if (ImGui::IsKeyPressed(ImGuiKey_C)) {
                editor->currentTool = EditorTool::Create;
            }
        }

        // Вікна інтерфейсу малюємо лише за умови showUi == true
        if (showUi) {
            // 3.1 ВЕРТИКАЛЬНА ПАНЕЛЬ ІНСТРУМЕНТІВ ЗЛІВА
            ImGui::SetNextWindowPos(ImVec2(padding, menuBarOffset + padding), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(48, maxAvailableHeight), ImGuiCond_Always);
            ImGui::Begin("##Toolbar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
            
            EditorTool tools[] = { EditorTool::Select, EditorTool::Create };
            const char* icons[] = { "[S]", "[C]" };
            const char* tooltips[] = { "Select & Transform (Hold Ctrl for quick-select)\nShortcut: S", "Obstacle Creation Tool\nShortcut: C" };

            for (int i = 0; i < 2; i++) {
                bool isActive = (editor->currentTool == tools[i]);
                if (isActive) {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.35f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.5f, 0.35f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                }
                
                if (ImGui::Button(icons[i], ImVec2(32, 32))) {
                    editor->currentTool = tools[i];
                }
                ImGui::PopStyleColor(2);

                if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", tooltips[i]);
                ImGui::Spacing();
            }
            ImGui::End();

            // 3.2 MAIN EDITOR WINDOW
            float editorWidth = 360.0f; 
            ImGui::SetNextWindowPos(ImVec2(screenW - editorWidth - padding, menuBarOffset + padding), resetEditorWorkspace ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(editorWidth, maxAvailableHeight), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSizeConstraints(ImVec2(340, 420), ImVec2(600, maxAvailableHeight));

            ImGui::Begin("Editor Workspace", NULL);

            if (resetEditorWorkspace) resetEditorWorkspace = false;

            // === ОБМЕЖЕННЯ РОБОЧОЇ ОБЛАСТІ РЕДАКТОРА (З УРАХУВАННЯМ ПАНЕЛІ ІНСТРУМЕНТІВ) ===
            ImVec2 posEd = ImGui::GetWindowPos();
            ImVec2 sizeEd = ImGui::GetWindowSize();
            ImVec2 clampedPosEd = posEd;

            float minAllowedX = padding + 48.0f + padding; 

            if (clampedPosEd.x < minAllowedX) clampedPosEd.x = minAllowedX;
            if (clampedPosEd.x + sizeEd.x > screenW - padding) clampedPosEd.x = screenW - sizeEd.x - padding;
            if (clampedPosEd.y < menuBarOffset + padding) clampedPosEd.y = menuBarOffset + padding;
            if (clampedPosEd.y + sizeEd.y > screenH - padding) clampedPosEd.y = screenH - sizeEd.y - padding;

            if (clampedPosEd.x != posEd.x || clampedPosEd.y != posEd.y) {
                ImGui::SetWindowPos(clampedPosEd);
            }

            ImGui::TextColored(UiTheme::ColorNormal, "[ Outliner ]");
            ImGui::BeginChild("HierarchyTree", ImVec2(0, 130), true);

            if (robot) {
                bool flashActive = (flashTimer > 0.0f && flashTarget == robot);
                if (flashActive) ImGui::PushStyleColor(ImGuiCol_Text, UiTheme::ColorCritical);
                
                if (ImGui::Selectable("System_Robot", currentSelected == robot)) {
                    editor->setSelectedEntity(robot);
                }
                
                if (flashActive) ImGui::PopStyleColor();
            }
            
            if (env) {
                bool flashActive = (flashTimer > 0.0f && flashTarget == env);
                if (flashActive) ImGui::PushStyleColor(ImGuiCol_Text, UiTheme::ColorCritical);
                
                if (ImGui::Selectable("System_Arena (Environment)", currentSelected == env)) {
                    editor->setSelectedEntity(env);
                }
                
                if (flashActive) ImGui::PopStyleColor();
            }
            
            ImGui::Separator();
            for (size_t i = 0; i < obstacles.size(); ++i) {
                std::string name = obstacles[i]->getTypeName() + "_" + std::to_string(i);
                if (ImGui::Selectable(name.c_str(), currentSelected == obstacles[i].get())) {
                    editor->setSelectedEntity(obstacles[i].get());
                }
            }
            ImGui::EndChild();

            ImGui::Separator();
            ImGui::Spacing();

            // Інспектор активного виділеного об'єкта
            if (currentSelected != nullptr) {
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(UiTheme::ColorAccent, "[ Active Inspector ]");
                ImGui::SameLine(ImGui::GetWindowWidth() - 85);
                if (ImGui::Button("Deselect", ImVec2(75, 20))) {
                    editor->setSelectedEntity(nullptr);
                    currentSelected = nullptr;
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Shortcut: Ctrl+D");
                }

                if (currentSelected != nullptr) {
                    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Spacing();
                        
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Position:"); ImGui::SameLine(controlAlignX);
                        ImGui::Text("X"); ImGui::SameLine();
                        ImGui::SetNextItemWidth(80);
                        
                        bool posChanged = false;
                        posChanged |= ImGui::DragFloat("##PosX", &currentSelected->entityPos.x, 1.0f, -2000.0f, 4000.0f, "%.1f");
                        
                        ImGui::SameLine();
                        ImGui::Text("Y"); ImGui::SameLine();
                        ImGui::SetNextItemWidth(80);
                        posChanged |= ImGui::DragFloat("##PosY", &currentSelected->entityPos.y, 1.0f, -2000.0f, 4000.0f, "%.1f");

                        if (posChanged) {
                            scene->requestGridClear = true;
                        }

                        if (currentSelected == robot) {
                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("Velocity:"); ImGui::SameLine(controlAlignX);
                            ImGui::SetNextItemWidth(-1);
                            float tempRobotVelocity = robot->getVelocity();
                            if (ImGui::SliderFloat("##Vel", &tempRobotVelocity, ROBOT_VELOCITY_MIN, ROBOT_VELOCITY_MAX, "%.1f")) {
                                robot->setVelocity(tempRobotVelocity);
                            }

                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("Radius:"); ImGui::SameLine(controlAlignX);
                            ImGui::SetNextItemWidth(-1);
                            float tempRobotRadius = robot->getRadius();
                            if (ImGui::SliderFloat("##Rad", &tempRobotRadius, ROBOT_RADIUS_MIN, ROBOT_RADIUS_MAX, "%.1f")) {
                                robot->setRadius(tempRobotRadius);
                            }
                        }
                        else if (currentSelected == env) {
                            float tempW = env->width;
                            float tempH = env->height;
                            bool sizeChanged = false;

                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("Arena Width:"); ImGui::SameLine(controlAlignX);
                            ImGui::SetNextItemWidth(-1);
                            sizeChanged |= ImGui::SliderFloat("##Width",  &tempW, ARENA_SIZE_MIN, ARENA_SIZE_MAX, "%.1f");

                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("Arena Height:"); ImGui::SameLine(controlAlignX);
                            ImGui::SetNextItemWidth(-1);
                            sizeChanged |= ImGui::SliderFloat("##Height", &tempH, ARENA_SIZE_MIN, ARENA_SIZE_MAX, "%.1f");
                            
                            if (sizeChanged) {
                                env->setDimensions(tempW, tempH);
                                scene->requestGridClear = true;
                            }
                        }
                        else if (auto* circle = dynamic_cast<CircleObstacle*>(currentSelected)) {
                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("Radius:"); ImGui::SameLine(controlAlignX);
                            ImGui::SetNextItemWidth(-1);
                            float r = circle->getRadius();
                            if (ImGui::SliderFloat("##CircleRad", &r, CIRCLE_RADIUS_MIN, CIRCLE_RADIUS_MAX, "%.1f")) {
                                circle->setRadius(r);
                                scene->requestGridClear = true;
                            }
                        }
                        else if (auto* rect = dynamic_cast<RectObstacle*>(currentSelected)) {
                            bool changed = false;
                            float w = rect->getWidth();
                            float h = rect->getHeight();
                            
                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("Width:"); ImGui::SameLine(controlAlignX);
                            ImGui::SetNextItemWidth(-1);
                            changed |= ImGui::SliderFloat("##RectW", &w, RECT_SIZE_MIN, RECT_SIZE_MAX, "%.1f");

                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("Height:"); ImGui::SameLine(controlAlignX);
                            ImGui::SetNextItemWidth(-1);
                            changed |= ImGui::SliderFloat("##RectH", &h, RECT_SIZE_MIN, RECT_SIZE_MAX, "%.1f");
                            
                            if (changed) {
                                rect->setDimensions(w, h);
                                scene->requestGridClear = true;
                            }
                        }
                        ImGui::Spacing();
                    }

                    if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Spacing();
                        
                        if (currentSelected == env) {
                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("Outline Color:"); ImGui::SameLine(controlAlignX);
                            ImGui::SetNextItemWidth(-1);
                            ImGui::ColorEdit4("##OutlineTint", &env->style.outlineColor.r);

                            ImGui::AlignTextToFramePadding();
                            ImGui::Text("Outline Width:"); ImGui::SameLine(controlAlignX);
                            ImGui::SetNextItemWidth(-1);
                            ImGui::SliderFloat("##LineWidth", &env->style.lineWidth, 1.0f, 5.0f, "%.1f");
                        } 
                        else {
                            RenderStyle* activeStyle = (currentSelected == robot) ? &robot->style : &static_cast<Obstacle*>(currentSelected)->style;
                            if (activeStyle) {
                                ImGui::AlignTextToFramePadding();
                                ImGui::Text("Render Mode:"); ImGui::SameLine(controlAlignX);
                                ImGui::SetNextItemWidth(-1);
                                int geomMode = static_cast<int>(activeStyle->mode);
                                if (ImGui::Combo("##DrawMode", &geomMode, "Outline\0Fill\0FillAndOutline\0")) {
                                    activeStyle->mode = static_cast<DrawMode>(geomMode);
                                }

                                ImGui::AlignTextToFramePadding();
                                ImGui::Text("Fill Color:"); ImGui::SameLine(controlAlignX);
                                ImGui::SetNextItemWidth(-1);
                                ImGui::ColorEdit4("##FillTint", &activeStyle->fillColor.r);

                                ImGui::AlignTextToFramePadding();
                                ImGui::Text("Outline Color:"); ImGui::SameLine(controlAlignX);
                                ImGui::SetNextItemWidth(-1);
                                ImGui::ColorEdit4("##OutlineTint", &activeStyle->outlineColor.r);

                                ImGui::AlignTextToFramePadding();
                                ImGui::Text("Outline Width:"); ImGui::SameLine(controlAlignX);
                                ImGui::SetNextItemWidth(-1);
                                ImGui::SliderFloat("##LineWidth", &activeStyle->lineWidth, 1.0f, 5.0f, "%.1f");
                            }
                        }
                        ImGui::Spacing();
                    }

                    if (currentSelected != robot && currentSelected != env) {
                        ImGui::Spacing();
                        ImGui::Separator();
                        if (ImGui::Button("Delete Object", ImVec2(-1, 25))) {
                            Renderer::getInstance()->freeEntityMesh(currentSelected);
                            env->removeObstacle(static_cast<Obstacle*>(currentSelected));
                            editor->setSelectedEntity(nullptr);
                            currentSelected = nullptr;
                            scene->requestGridClear = true;
                        }
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Shortcut: Delete");
                        }
                    }
                }
            } 
            else if (editor->currentTool == EditorTool::Create) {
                ImGui::TextColored(UiTheme::ColorNormal, "[ Placement Options ]");
                
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Obstacle Type:"); ImGui::SameLine(controlAlignX);
                ImGui::RadioButton("Circle", &editor->spawnObjectType, 0); ImGui::SameLine();
                ImGui::RadioButton("Rectangle", &editor->spawnObjectType, 1);
                ImGui::Separator();

                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Spacing();
                    if (editor->spawnObjectType == 0) {
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Radius:"); ImGui::SameLine(controlAlignX);
                        ImGui::SetNextItemWidth(-1);
                        ImGui::SliderFloat("##RadiusTool", &editor->newCircleRadius, CIRCLE_RADIUS_MIN, CIRCLE_RADIUS_MAX, "%.1f");
                    } else {
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Width:"); ImGui::SameLine(controlAlignX);
                        ImGui::SetNextItemWidth(-1);
                        ImGui::SliderFloat("##WidthTool",  &editor->newRectWidth,  RECT_SIZE_MIN, RECT_SIZE_MAX, "%.1f");

                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Height:"); ImGui::SameLine(controlAlignX);
                        ImGui::SetNextItemWidth(-1);
                        ImGui::SliderFloat("##HeightTool", &editor->newRectHeight, RECT_SIZE_MIN, RECT_SIZE_MAX, "%.1f");
                    }
                    ImGui::Spacing();
                }

                if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Spacing();
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Render Mode:"); ImGui::SameLine(controlAlignX);
                    ImGui::SetNextItemWidth(-1);
                    ImGui::Combo("##DrawModeTool", &editor->newDrawMode, "Outline\0Fill\0FillAndOutline\0");

                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Fill Color:"); ImGui::SameLine(controlAlignX);
                    ImGui::SetNextItemWidth(-1);
                    ImGui::ColorEdit4("##FillColorTool", editor->newFillColor);

                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Outline Color:"); ImGui::SameLine(controlAlignX);
                    ImGui::SetNextItemWidth(-1);
                    ImGui::ColorEdit4("##OutlineColorTool", editor->newOutlineColor);

                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Outline Width:"); ImGui::SameLine(controlAlignX);
                    ImGui::SetNextItemWidth(-1);
                    ImGui::SliderFloat("##LineWidthTool", &editor->newLineWidth, 1.0f, 5.0f, "%.1f");
                    ImGui::Spacing();
                }
                
                ImGui::Spacing();
                ImGui::TextColored(UiTheme::ColorImportant, "* Left Click map workspace to add object.");
            } 
            else {
                ImGui::TextUnformatted("Select an entity from Outliner\nor use the Object Tool to design maps.");
            }
            ImGui::End();
        }

        // === Gizmo ===
        currentSelected = editor->getSelectedEntity();

        if (currentSelected != nullptr) {
            glm::vec2 minBounds(0.0f), maxBounds(0.0f);
            
            if (currentSelected->getBounds(minBounds, maxBounds)) {
                float minX = minBounds.x, minY = minBounds.y;
                float maxX = maxBounds.x, maxY = maxBounds.y;

                ImDrawList* drawList = ImGui::GetForegroundDrawList();
                ImVec2 mousePos = ImGui::GetMousePos();

                ImU32 blueAccent = IM_COL32(0, 120, 215, 255);
                ImU32 handleColor = IM_COL32(255, 255, 255, 255);

                drawList->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), blueAccent, 0.0f, 0, 1.5f);

                ImVec2 handlePos(maxX, maxY);

                ImGui::SetNextWindowPos(ImVec2(handlePos.x - 6, handlePos.y - 6));
                ImGui::SetNextWindowSize(ImVec2(12, 12));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::Begin("##GizmoHandleWin", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
                
                ImGui::InvisibleButton("##GizmoHandleBtn", ImVec2(12, 12));
                bool isOverHandle = ImGui::IsItemHovered();
                bool isDraggingGizmo = ImGui::IsItemActive();
                
                ImGui::End();
                ImGui::PopStyleVar();

                if (isDraggingGizmo) {
                    currentSelected->resizeByGizmo(glm::vec2(mousePos.x, mousePos.y));
                    scene->requestGridClear = true;
                }

                if (isDraggingGizmo)     handleColor = IM_COL32(0, 100, 180, 255);
                else if (isOverHandle)   handleColor = IM_COL32(0, 140, 240, 255);

                drawList->AddRectFilled(ImVec2(handlePos.x - 4, handlePos.y - 4), ImVec2(handlePos.x + 4, handlePos.y + 4), handleColor);
                drawList->AddRect(ImVec2(handlePos.x - 4, handlePos.y - 4), ImVec2(handlePos.x + 4, handlePos.y + 4), blueAccent, 0.0f, 0, 1.0f);
            }
        }
    }
}