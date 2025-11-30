#include "InputManager.h"

glm::vec2 InputManager::getMovementDirection(GLFWwindow* window) {
    glm::vec2 dir(0.0f);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) dir.y -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) dir.y += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) dir.x -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) dir.x += 1.0f;
    
    // Нормалізація вектора, щоб рух по діагоналі не був швидшим
    if (dir.x != 0.f || dir.y != 0.f) {
        dir = glm::normalize(dir);
    }

    return dir;
}

bool InputManager::isKeyPressed(GLFWwindow* window, int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}