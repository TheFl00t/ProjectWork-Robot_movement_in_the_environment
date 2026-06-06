#pragma once

#include "../config.h"
#include "../json.hpp"
#include "Scene.h"

using json = nlohmann::json;

class ConfigLoader {
private:
    static std::string drawModeToString(DrawMode mode);
    static DrawMode stringToDrawMode(const std::string& str);

    static glm::vec4 parseColor(const json& colorData);
public:
    static std::string getConfigPath(const std::string& fileName, bool showLog = true);
    
    // Завантажує лише розміри вікна
    static void loadWindowSize(const std::string& fileName, int& width, int& height);
    // Створює та повертає готову сцену з усіма об'єктами
    static Scene* loadScene(const std::string& fileName, int winWidth, int winHeight);
    // Зберігає сцену у сейв/конфіг файл
    static void saveScene(const std::string& fileName, Scene* scene, int winWidth, int winHeight);
};