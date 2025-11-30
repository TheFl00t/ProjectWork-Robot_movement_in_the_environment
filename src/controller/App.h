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
    GLFWwindow* window;
    Robot* robot;
    Renderer* renderer;
    
    bool resetGuiPos = false;

    // Обробка натискань клавіш
    void processInput();
    
    // Розрахунок часу між кадрами
    float computeDeltaTime();

public:
    // Головний метод запуску
    void run();
};