#include "MapEditor.h"
#include "../model/CircleObstacle.h"
#include "../model/RectObstacle.h"

void MapEditor::update(GLFWwindow* window, Scene* scene, glm::vec2 worldMousePos) {
    if (!scene) return;

    int leftMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    Environment* env = scene->getEnvironmentPointer();
    Robot* robot = scene->getRobot();

    // Перевіряємо затискання клавіші CTRL
    bool isCtrlPressed = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || 
                          glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

    EditorTool effectiveTool = isCtrlPressed ? EditorTool::Select : currentTool;
    
    // 1. НАТИСКАННЯ МИШІ
    if (leftMouseState == GLFW_PRESS && !wasLeftPressed) {
        wasLeftPressed = true;

        if (effectiveTool == EditorTool::Select) {
            if (robot && robot->containsPoint(worldMousePos)) {
                selectedEntity = robot;
                isDragging = true;
            } 
            else {
                Entity* clickedObstacle = nullptr;
                const auto& obstacles = env->getObstacles();
                for (int i = static_cast<int>(obstacles.size()) - 1; i >= 0; --i) {
                    if (obstacles[i]->containsPoint(worldMousePos)) {
                        clickedObstacle = obstacles[i].get();
                        break;
                    }
                }

                if (clickedObstacle) {
                    selectedEntity = clickedObstacle;
                    isDragging = true;
                } 
                else if (env && env->containsPoint(worldMousePos)) {
                    selectedEntity = env;
                    isDragging = true;
                } 
                else {
                    selectedEntity = nullptr; // Клік по порожньому місцю скидає фокус
                }
            }

            if (isDragging && selectedEntity) {
                dragOffset = selectedEntity->entityPos - worldMousePos;
            }
        } 
        else {
            // Режим створення об'єктів (працює по кліку, якщо CTRL відпущено)
            Obstacle* newObs = nullptr;
            if (spawnObjectType == 0) {
                newObs = new CircleObstacle(worldMousePos, newCircleRadius);
            } else if (spawnObjectType == 1) {
                newObs = new RectObstacle(worldMousePos, newRectWidth, newRectHeight);
            }

            if (newObs) {
                newObs->style.mode = static_cast<DrawMode>(newDrawMode);
                newObs->style.lineWidth = newLineWidth;
                newObs->style.fillColor = glm::vec4(newFillColor[0], newFillColor[1], newFillColor[2], newFillColor[3]);
                newObs->style.outlineColor = glm::vec4(newOutlineColor[0], newOutlineColor[1], newOutlineColor[2], newOutlineColor[3]);

                env->addObstacle(std::unique_ptr<Obstacle>(newObs));
                scene->requestGridClear = true;
                selectedEntity = newObs; // Фокусуємось на створеному об'єкті для редагування
            }
        }
    }
    // 2. УТРИМАННЯ МИШІ (Працює тільки для інструмента Select)
    else if (leftMouseState == GLFW_PRESS && wasLeftPressed) {
        if (isDragging && selectedEntity && effectiveTool == EditorTool::Select) {
            selectedEntity->entityPos = worldMousePos + dragOffset;
            scene->requestGridClear = true;
        }
    }
    // 3. ВІДПУСКАННЯ МИШІ
    else if (leftMouseState == GLFW_RELEASE) {
        wasLeftPressed = false;
        isDragging = false;
    }
}