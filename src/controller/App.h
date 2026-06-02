#pragma once

// Підключення моделей
#include "../model/Robot.h"
#include "../model/CircleObstacle.h"
#include "../model/RectObstacle.h"
#include "../model/Point.h"

// Підключення відображення
#include "../view/Renderer.h"

// Підключення контролерів
#include "InputManager.h"
#include "Scene.h"

class App {
private:
    GLFWwindow* window = nullptr;
    Robot* robot = nullptr;
    Scene* scene = nullptr;
    Renderer* renderer = nullptr;
    
    bool resetGuiPos = false;

    // Обробка натискань клавіш
    void processInput();
    
    // Розрахунок часу між кадрами
    float computeDeltaTime();

    glm::vec2 getScreenToWorldMousePos();

    // Состояние редактора
    bool isEditMode = false;
    int selectedObstacleType = 0; // 0 - Круг, 1 - Прямоугольник

    // Параметры нового объекта
    float newCircleRadius = 40.0f;
    float newRectWidth = 80.0f;
    float newRectHeight = 50.0f;

    // Параметры стиля нового объекта
    int newDrawMode = 2; // 0 - Outline, 1 - Fill, 2 - FillAndOutline
    float newFillColor[4] = { 0.5f, 0.5f, 0.5f, 0.4f };
    float newOutlineColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float newLineWidth = 1.5f;
    
public:
    // Головний метод запуску
    void run();
};