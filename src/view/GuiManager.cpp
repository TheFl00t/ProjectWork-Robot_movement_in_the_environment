#include "GuiManager.h"
#include "imgui.h"
#include "../controller/ConfigLoader.h"
#include "../view/Renderer.h"

#include "../model/Robot.h"
#include "../model/Environment.h"
#include "../model/CircleObstacle.h"
#include "../model/RectObstacle.h"

#include <iostream>
#include <algorithm>

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

void GuiManager::render(GLFWwindow* window, Scene*& scene, MapEditor* editor, AppState& currentState, int& winWidth, int& winHeight) {
    float padding = 10.0f;
    float menuBarOffset = 25.0f;
    std::string configPath = ConfigLoader::getConfigPath("config.json", false);

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

    // ============================================================
    // 1. MAIN MENU BAR
    // ============================================================
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Config File")) {
                ShellExecuteA(NULL, "open", "notepad.exe", configPath.c_str(), NULL, SW_SHOW);
            }
            if (ImGui::MenuItem("Save Active Map", "Ctrl+S")) {
                ConfigLoader::saveScene(configPath, scene, winWidth, winHeight);
            }
            if (ImGui::MenuItem("Reload from Config File")) {
                Scene* newScene = ConfigLoader::loadScene(configPath, winWidth, winHeight);
                if (newScene != nullptr) { 
                    delete scene; 
                    scene = newScene; 
                    editor->setSelectedEntity(nullptr);
                }
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
        
        // Horizontal padding for mode selectors
        ImGui::Spacing(); ImGui::SameLine(0, 15);
        ImGui::Text("App Mode:"); 
        
        ImGui::SameLine(0, 15);
        bool isEditMode = (currentState == AppState::Editor);
        if (ImGui::RadioButton("Simulation", !isEditMode)) {
            currentState = AppState::Simulation;
        }
        
        ImGui::SameLine(0, 15);
        if (ImGui::RadioButton("Map Editor", isEditMode)) {
            currentState = AppState::Editor;
            if (scene && scene->getRobot()) scene->getRobot()->direction = glm::vec2(0.f);
        }

        ImGui::SameLine(0, 40);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "|   FPS: %.1f", ImGui::GetIO().Framerate);

        ImGui::EndMainMenuBar();
    }

    // Якщо інтерфейс приховано через TAB або меню Window - не рендеримо бічні панелі
    if (!showUi) return;

    // ============================================================
    // 2. SIMULATION WINDOW (Simulation Mode)
    // ============================================================
    if (currentState == AppState::Simulation) {
        float controlWidth = 320.0f;
        
        ImGui::SetNextWindowPos(ImVec2(screenW - controlWidth - padding, menuBarOffset + padding), resetControlPanel ? ImGuiCond_Always : ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(controlWidth, 280), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(ImVec2(280, 180), ImVec2(400, maxAvailableHeight));
        
        ImGui::Begin("Control Panel", NULL, ImGuiWindowFlags_None);

        if (resetControlPanel) resetControlPanel = false;
        
        // --- ОБМЕЖЕННЯ ВІКНА ПАНЕЛІ КЕРУВАННЯ В МЕЖАХ ЕКРАНА ---
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        ImVec2 clampedPos = pos;

        if (clampedPos.x < padding) clampedPos.x = padding;
        if (clampedPos.x + size.x > screenW - padding) clampedPos.x = screenW - size.x - padding;
        if (clampedPos.y < menuBarOffset + padding) clampedPos.y = menuBarOffset + padding;
        if (clampedPos.y + size.y > screenH - padding) clampedPos.y = screenH - size.y - padding;

        if (clampedPos.x != pos.x || clampedPos.y != pos.y) {
            ImGui::SetWindowPos(clampedPos);
        }
        
        Robot* robot = scene ? scene->getRobot() : nullptr;
        if (robot) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[ Information ]");
            ImGui::Text("Robot Position: (%.1f, %.1f)", robot->entityPos.x, robot->entityPos.y);
            ImGui::Spacing();
            
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "[ Movement Physics ]");
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Velocity:"); ImGui::SameLine(100);
            ImGui::SetNextItemWidth(-1);
            float tempRobotVelocity = robot->getVelocity();
            if (ImGui::SliderFloat("##Velocity", &tempRobotVelocity, 0.0f, 600.0f, "%.1f")) {
                robot->setVelocity(tempRobotVelocity);
            }
            
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Radius:"); ImGui::SameLine(100);
            ImGui::SetNextItemWidth(-1);
            float tempRobotRadius = robot->getRadius();
            if (ImGui::SliderFloat("##Radius", &tempRobotRadius, 5.0f, 100.0f, "%.1f")) {
                robot->setRadius(tempRobotRadius);
            }
        }
        ImGui::Separator();

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[ Debug Settings ]");
        ImGui::Checkbox("Collision Point", &scene->showCollisionPoint);
        ImGui::Checkbox("Velocity Vector", &scene->showVelocityVector);

        ImGui::Spacing();
        if (ImGui::Button("Reset Robot Position", ImVec2(-1, 25))) {
            if (robot) {
                robot->entityPos = robot->startPos;
                robot->direction = glm::vec2(0.f);
                scene->checkCollision(robot->entityPos);
            }
        }
        if (ImGui::Button("Reset Robot Velocity", ImVec2(-1, 25))) {
            if (robot) {
                robot->setVelocity(robot->startVelocity);
                scene->checkCollision(robot->entityPos);
            }
        }
        if (ImGui::Button("Reset Robot Radius", ImVec2(-1, 25))) {
            if (robot) {
                robot->setRadius(robot->startRadius);
                scene->checkCollision(robot->entityPos);
            }
        }
        ImGui::End();
    }

    // ============================================================
    // 3. ІНТЕРФЕЙС РЕДАКТОРА КАРТИ (Тільки в режимі редактора)
    // ============================================================
    if (currentState == AppState::Editor && editor && scene) {
        Environment* env = scene->getEnvironmentPointer();
        Robot* robot = scene->getRobot();
        const auto& obstacles = env->getObstacles();
        Entity* currentSelected = editor->getSelectedEntity();

        // ОБРОБКА ГАРЯЧОЇ КЛАВІШІ DELETE
        if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Delete) && currentSelected) {
            if (currentSelected == robot || currentSelected == env) {
                flashTimer = 0.4f; // Вмикаємо червоний спалах на 400мс
                flashTarget = currentSelected;
            } else {
                env->removeObstacle(static_cast<Obstacle*>(currentSelected));
                editor->setSelectedEntity(nullptr);
                currentSelected = nullptr;
            }
        }

        // 3.1 ВЕРТИКАЛЬНА ПАНЕЛЬ ІНСТРУМЕНТІВ ЗЛІВА
        ImGui::SetNextWindowPos(ImVec2(padding, menuBarOffset + padding), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(48, maxAvailableHeight), ImGuiCond_Always);
        ImGui::Begin("##Toolbar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
        
        EditorTool tools[] = { EditorTool::Select, EditorTool::Create };
        const char* icons[] = { "[S]", "[C]" };
        const char* tooltips[] = { "Select & Transform (Hold Ctrl for quick-select)", "Obstacle Creation Tool" };

        // ОБРОБКА ГАРЯЧИХ КЛАВІШ [S, C]
        if (!ImGui::GetIO().WantTextInput) {
            if (ImGui::IsKeyPressed(ImGuiKey_S)) {
                editor->currentTool = EditorTool::Select;
            } else if (ImGui::IsKeyPressed(ImGuiKey_C)) {
                editor->currentTool = EditorTool::Create;
            }
        }

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

        // --- ОБМЕЖЕННЯ РОБОЧОЇ ОБЛАСТІ РЕДАКТОРА (З УРАХУВАННЯМ ПАНЕЛІ ІНСТРУМЕНТІВ) ---
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

        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.8f, 1.0f), "[ Outliner ]");
        ImGui::BeginChild("HierarchyTree", ImVec2(0, 130), true);

        if (robot) {
            // Ефект спалаху для Робота
            bool flashActive = (flashTimer > 0.0f && flashTarget == robot);
            if (flashActive) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            
            if (ImGui::Selectable("System_Robot", currentSelected == robot)) {
                editor->setSelectedEntity(robot);
            }
            
            if (flashActive) ImGui::PopStyleColor();
        }
        
        if (env) {
            // Ефект спалаху для Арени
            bool flashActive = (flashTimer > 0.0f && flashTarget == env);
            if (flashActive) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            
            if (ImGui::Selectable("System_Arena (Environment)", currentSelected == env)) {
                editor->setSelectedEntity(env);
            }
            
            if (flashActive) ImGui::PopStyleColor();
        }
        
        ImGui::Separator();
        for (size_t i = 0; i < obstacles.size(); ++i) {

            std::string name = obstacles[i]->getTypeName() + "_" + std::to_string(i);
            if (ImGui::Selectable(name.c_str(), currentSelected == obstacles[i])) {
                editor->setSelectedEntity(obstacles[i]);
            }
        }
        ImGui::EndChild();

        ImGui::Separator();
        ImGui::Spacing();

        const float controlAlignX = 115.0f;

        // СЦЕНАРІЙ А: Інспектор активного виділеного об'єкта
        if (currentSelected != nullptr) {
            
            // ВПРОВАДЖЕННЯ КНОПКИ DESELECT У РЯДОК ЗАГОЛОВКА
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "[ Active Inspector ]");
            ImGui::SameLine(ImGui::GetWindowWidth() - 85);
            if (ImGui::Button("Deselect", ImVec2(75, 20))) {
                editor->setSelectedEntity(nullptr);
                currentSelected = nullptr;
            }

            if (currentSelected != nullptr) { // Перевіряємо повторно на випадок, якщо щойно натиснули Deselect
                // --- TRANSFORM ---
                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Spacing();
                    
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Position:"); ImGui::SameLine(controlAlignX);
                    ImGui::Text("X"); ImGui::SameLine();
                    ImGui::SetNextItemWidth(80);
                    ImGui::DragFloat("##PosX", &currentSelected->entityPos.x, 1.0f, -2000.0f, 4000.0f, "%.1f");
                    
                    ImGui::SameLine();
                    ImGui::Text("Y"); ImGui::SameLine();
                    ImGui::SetNextItemWidth(80);
                    ImGui::DragFloat("##PosY", &currentSelected->entityPos.y, 1.0f, -2000.0f, 4000.0f, "%.1f");

                    if (currentSelected == robot) {
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Velocity:"); ImGui::SameLine(controlAlignX);
                        ImGui::SetNextItemWidth(-1);
                        float tempRobotVelocity = robot->getVelocity();
                        if (ImGui::SliderFloat("##Vel", &tempRobotVelocity, 0.0f, 600.0f, "%.1f")) {
                            robot->setVelocity(tempRobotVelocity);
                        }

                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Radius:"); ImGui::SameLine(controlAlignX);
                        ImGui::SetNextItemWidth(-1);
                        float tempRobotRadius = robot->getRadius();
                        if (ImGui::SliderFloat("##Rad", &tempRobotRadius, 5.0f, 100.0f, "%.1f")) {
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
                        sizeChanged |= ImGui::SliderFloat("##Width", &tempW, 200.0f, 2048.0f, "%.1f");

                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Arena Height:"); ImGui::SameLine(controlAlignX);
                        ImGui::SetNextItemWidth(-1);
                        sizeChanged |= ImGui::SliderFloat("##Height", &tempH, 200.0f, 2048.0f, "%.1f");
                        
                        if (sizeChanged && env->getMesh()) {
                            env->setDimensions(tempW, tempH);
                        }
                    }
                    else if (auto* circle = dynamic_cast<CircleObstacle*>(currentSelected)) {
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Radius:"); ImGui::SameLine(controlAlignX);
                        ImGui::SetNextItemWidth(-1);
                        if (ImGui::SliderFloat("##CircleRad", &circle->radius, 5.0f, 200.0f, "%.1f")) {
                            circle->updateMesh();
                        }
                    } 
                    else if (auto* rect = dynamic_cast<RectObstacle*>(currentSelected)) {
                        bool changed = false;
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Width:"); ImGui::SameLine(controlAlignX);
                        ImGui::SetNextItemWidth(-1);
                        changed |= ImGui::SliderFloat("##RectW", &rect->width, 10.0f, 500.0f, "%.1f");

                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Height:"); ImGui::SameLine(controlAlignX);
                        ImGui::SetNextItemWidth(-1);
                        changed |= ImGui::SliderFloat("##RectH", &rect->height, 10.0f, 500.0f, "%.1f");
                        
                        if (changed) rect->updateMesh(); 
                    }
                    ImGui::Spacing();
                }

                // --- APPEARANCE ---
                if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Spacing();
                    
                    if (currentSelected == env) {
                        // Безпечний вивід для Арени: тільки колір рамок та товщина ліній (без Fill)
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
                        // Стандартний вивід для всіх інших перешкод та робота
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
                        env->removeObstacle(static_cast<Obstacle*>(currentSelected));
                        editor->setSelectedEntity(nullptr);
                    }
                }
            }
        } 
        // СЦЕНАРІЙ Б: Налаштування розміщення нових об'єктів
        else if (editor->currentTool == EditorTool::Create) {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "[ Placement Options ]");
            
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
                    ImGui::SliderFloat("##RadiusTool", &editor->newCircleRadius, 10.0f, 150.0f, "%.1f");
                } else {
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Width:"); ImGui::SameLine(controlAlignX);
                    ImGui::SetNextItemWidth(-1);
                    ImGui::SliderFloat("##WidthTool", &editor->newRectWidth, 10.0f, 300.0f, "%.1f");

                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Height:"); ImGui::SameLine(controlAlignX);
                    ImGui::SetNextItemWidth(-1);
                    ImGui::SliderFloat("##HeightTool", &editor->newRectHeight, 10.0f, 300.0f, "%.1f");
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
            ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "* Left Click map workspace to add object.");
        } 
        else {
            ImGui::TextUnformatted("Select an entity from Outliner\nor use the Object Tool to design maps.");
        }

        ImGui::End();
    }
}