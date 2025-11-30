#pragma once

#include "../config.h"
#include "Scene.h"

class ConfigLoader {
public:
    static std::string getConfigPath(const std::string& fileName);
    
    // Завантажує лише розміри вікна
    static void loadWindowSize(const std::string& fileName, int& width, int& height);

    // Створює та повертає готову сцену з усіма об'єктами
    static Scene* loadScene(const std::string& fileName, int winWidth, int winHeight);
};