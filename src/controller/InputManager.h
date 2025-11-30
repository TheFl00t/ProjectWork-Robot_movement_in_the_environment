#pragma once

#include "../config.h"

class InputManager {
public:
    // Повертає вектор напрямку руху на основі WASD (нормалізований)
    static glm::vec2 getMovementDirection(GLFWwindow* window);

    // Перевіряє, чи натиснута конкретна клавіша
    static bool isKeyPressed(GLFWwindow* window, int key);
};