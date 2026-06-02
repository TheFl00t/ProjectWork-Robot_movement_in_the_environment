#include "MapEditor.h"
#include "../model/CircleObstacle.h"
#include "../model/RectObstacle.h"
#include <iostream>

void MapEditor::update(GLFWwindow* window, Scene* scene, glm::vec2 worldMousePos) {
    static bool wasLeftPressed = false;
    int leftMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    if (leftMouseState == GLFW_PRESS && !wasLeftPressed) {
        wasLeftPressed = true;

        if (scene) {
            Obstacle* newObstacle = nullptr;

            if (selectedObstacleType == 0) {
                newObstacle = new CircleObstacle(worldMousePos, newCircleRadius);
            } else {
                newObstacle = new RectObstacle(worldMousePos, newRectWidth, newRectHeight);
            }

            if (newObstacle) {
                newObstacle->style.mode = static_cast<DrawMode>(newDrawMode);
                newObstacle->style.lineWidth = newLineWidth;
                newObstacle->style.fillColor = glm::vec4(newFillColor[0], newFillColor[1], newFillColor[2], newFillColor[3]);
                newObstacle->style.outlineColor = glm::vec4(newOutlineColor[0], newOutlineColor[1], newOutlineColor[2], newOutlineColor[3]);

                scene->getEnvironmentPointer()->addObstacle(newObstacle);
                std::cout << "[Editor] Spawned new obstacle at: (" << worldMousePos.x << ", " << worldMousePos.y << ")" << std::endl;
            }
        }
    } 
    else if (leftMouseState == GLFW_RELEASE) {
        wasLeftPressed = false;
    }
}