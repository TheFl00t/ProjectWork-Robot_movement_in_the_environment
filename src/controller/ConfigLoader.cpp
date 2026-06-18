#include "ConfigLoader.h"
#include "../model/CircleObstacle.h"
#include "../model/RectObstacle.h"

namespace fs = std::filesystem;

std::string ConfigLoader::getConfigPath(const std::string& fileName, bool showLog) {
    std::string path = std::filesystem::absolute(fileName).string();
    if (showLog) {
        std::cout << "[ConfigLoader] Absolute path to config: " << path << std::endl;
    }
    return path;
}

std::string ConfigLoader::drawModeToString(DrawMode mode) {
    if (mode == DrawMode::Fill) return "Fill";
    if (mode == DrawMode::FillAndOutline) return "FillAndOutline";
    return "Outline";
}

DrawMode ConfigLoader::stringToDrawMode(const std::string& str) {
    if (str == "Fill") return DrawMode::Fill;
    if (str == "FillAndOutline") return DrawMode::FillAndOutline;
    return DrawMode::Outline;
}

glm::vec4 ConfigLoader::parseColor(const json& colorData) {
    glm::vec4 color(1.0f); // білий за замовчуванням

    // Варіант 1: Користувач передав масив [R, G, B, A] або [R, G, B] у флоатах
    if (colorData.is_array() && colorData.size() >= 3) {
        color.r = colorData[0];
        color.g = colorData[1];
        color.b = colorData[2];
        color.a = (colorData.size() == 4) ? static_cast<float>(colorData[3]) : 1.0f;
    }
    // Варіант 2: Користувач передав рядок виду "#ffffff" або "#ffffffaa"
    else if (colorData.is_string()) {
        std::string hexStr = colorData.get<std::string>();
        if (!hexStr.empty() && hexStr[0] == '#') {
            hexStr = hexStr.substr(1); // Вирізаємо символ '#'

            // довжина 6 = RGB, 8 = RGBA
            if (hexStr.length() == 6 || hexStr.length() == 8) {
                try {
                    // hex -> 0..255
                    unsigned int r = std::stoul(hexStr.substr(0, 2), nullptr, 16);
                    unsigned int g = std::stoul(hexStr.substr(2, 2), nullptr, 16);
                    unsigned int b = std::stoul(hexStr.substr(4, 2), nullptr, 16);
                    unsigned int a = 255; // За замовчуванням непрозорий

                    if (hexStr.length() == 8) {
                        a = std::stoul(hexStr.substr(6, 2), nullptr, 16);
                    }

                    // -> 0.0..1.0
                    color = glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
                } catch (...) {
                    std::cerr << "[ConfigLoader] Invalid HEX color format: #" << hexStr << std::endl;
                }
            }
        }
    }
    return color;
}

void ConfigLoader::loadWindowSize(const std::string& fileName, int& width, int& height) {
    width = 1024; // якщо файл не знайшли
    height = 768;

    std::ifstream file(fileName);
    if (!file.is_open()) return;

    try {
        json data = json::parse(file);
        if (data.contains("window")) {
            width = data["window"]["width"];
            height = data["window"]["height"];
        }
    } catch (json::parse_error& e) {
        std::cerr << "[ConfigLoader] JSON parse error in loadWindowSize: " << e.what() << std::endl;
    }
}

Scene* ConfigLoader::loadScene(const std::string& fileName, int winWidth, int winHeight) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cerr << "[ConfigLoader] Could not open config file: " << fileName << ". Creating default scene." << std::endl;
        auto r = std::make_unique<Robot>(glm::vec2(winWidth * 0.5f, winHeight * 0.5f), 25.f, 200.f);
        auto env = std::make_unique<Environment>(glm::vec2(20.f, 20.f), winWidth - 40.f, winHeight - 40.f);
        return new Scene(std::move(r), std::move(env));
    }

    try {
        json data = json::parse(file);

        // 1. Завантаження параметрів робота
        auto rData = data["robot"];
        auto robot = std::make_unique<Robot>(
            glm::vec2(rData["position"][0], rData["position"][1]),
            rData["radius"],
            rData["velocity"]
        );
        if (rData.contains("style")) {
            robot->style.mode = stringToDrawMode(rData["style"]["draw_mode"]);
            robot->style.lineWidth = rData["style"]["line_width"];
            // Використовуємо універсальний парсер замість прямих індексів масиву
            robot->style.fillColor = parseColor(rData["style"]["fill_color"]);
            robot->style.outlineColor = parseColor(rData["style"]["outline_color"]);
        }

        // 2. Завантаження меж арени (Environment)
        auto aData = data["arena"];
        auto env = std::make_unique<Environment>(
            glm::vec2(aData["position"][0], aData["position"][1]),
            aData["width"],
            aData["height"]
        );

        // 3. Завантаження масиву перешкод
        if (data.contains("obstacles") && data["obstacles"].is_array()) {
            for (auto& obsData : data["obstacles"]) {
                std::string type = obsData["type"];
                glm::vec2 pos(obsData["position"][0], obsData["position"][1]);
                Obstacle* obstacle = nullptr;

                if (type == "circle") {
                    obstacle = new CircleObstacle(pos, obsData["radius"]);
                } else if (type == "rect") {
                    obstacle = new RectObstacle(pos, obsData["width"], obsData["height"]);
                }

                // Якщо об'єкт успішно створено, парсимо його індивідуальний стиль
                if (obstacle && obsData.contains("style")) {
                    obstacle->style.mode = stringToDrawMode(obsData["style"]["draw_mode"]);
                    obstacle->style.lineWidth = obsData["style"]["line_width"];
                    // Використовуємо універсальний парсер замість прямих індексів масиву
                    obstacle->style.fillColor = parseColor(obsData["style"]["fill_color"]);
                    obstacle->style.outlineColor = parseColor(obsData["style"]["outline_color"]);
                    env->addObstacle(std::unique_ptr<Obstacle>(obstacle));
                }
            }
        }

        return new Scene(std::move(robot), std::move(env));

    } catch (json::parse_error& e) {
        std::cerr << "[ConfigLoader] Fatal JSON parse error: " << e.what() << std::endl;
        std::cerr << "[ConfigLoader] Config text is corrupted. Creating a clean default scene..." << std::endl;

        auto r = std::make_unique<Robot>(glm::vec2(winWidth * 0.5f, winHeight * 0.5f), 25.f, 200.f);
        auto env = std::make_unique<Environment>(glm::vec2(20.f, 20.f), winWidth - 40.f, winHeight - 40.f);
        return new Scene(std::move(r), std::move(env));
    }
}

void ConfigLoader::saveScene(const std::string& fileName, Scene* scene, int winWidth, int winHeight) {
    if (!scene) return;

    json data;

    // Збереження конфігурації вікна
    data["window"]["width"] = winWidth;
    data["window"]["height"] = winHeight;

    // Збереження стану та стилю робота
    Robot* robot = scene->getRobot();
    data["robot"]["position"] = { robot->entityPos.x, robot->entityPos.y };
    data["robot"]["radius"] = robot->getRadius();
    data["robot"]["velocity"] = robot->getVelocity();
    data["robot"]["style"]["draw_mode"] = drawModeToString(robot->style.mode);
    data["robot"]["style"]["fill_color"] = { robot->style.fillColor.r, robot->style.fillColor.g, robot->style.fillColor.b, robot->style.fillColor.a };
    data["robot"]["style"]["outline_color"] = { robot->style.outlineColor.r, robot->style.outlineColor.g, robot->style.outlineColor.b, robot->style.outlineColor.a };
    data["robot"]["style"]["line_width"] = robot->style.lineWidth;

    // Збереження параметрів арени
    Environment* env = scene->getEnvironmentPointer();
    data["arena"]["position"] = { env->entityPos.x, env->entityPos.y };
    data["arena"]["width"] = env->width;
    data["arena"]["height"] = env->height;

    // Серіалізація всіх динамічних перешкод у масив JSON
    data["obstacles"] = json::array();
    for (const auto& obs : env->getObstacles()) {
        json obsData;
        obsData["position"] = { obs->entityPos.x, obs->entityPos.y };
        obsData["style"]["draw_mode"] = drawModeToString(obs->style.mode);
        obsData["style"]["fill_color"] = { obs->style.fillColor.r, obs->style.fillColor.g, obs->style.fillColor.b, obs->style.fillColor.a };
        obsData["style"]["outline_color"] = { obs->style.outlineColor.r, obs->style.outlineColor.g, obs->style.outlineColor.b, obs->style.outlineColor.a };
        obsData["style"]["line_width"] = obs->style.lineWidth;

        obs->serialize(obsData);

        data["obstacles"].push_back(obsData);
    }

    std::ofstream file(fileName);
    if (file.is_open()) {
        file << data.dump(2);
        std::cout << "[ConfigLoader] Scene successfully saved to " << fileName << std::endl;
    } else {
        std::cerr << "[ConfigLoader] Failed to open file for saving: " << fileName << std::endl;
    }
}

void ConfigLoader::saveAppSettings(const std::string& fileName, Scene* scene) {
    if (!scene || !scene->getRobot()) return;

    json data;
    Robot* robot = scene->getRobot();
    Lidar& lidar = robot->getLidar();

    // Налаштування лідара
    data["lidar"]["visible"] = lidar.visible;
    data["lidar"]["ray_count"] = lidar.getRayCount();
    data["lidar"]["max_distance"] = lidar.getMaxDistance();
    data["lidar"]["fov"] = lidar.getFov();
    data["lidar"]["color"] = { lidar.rayColor.r, lidar.rayColor.g, lidar.rayColor.b, lidar.rayColor.a };

    // Налаштування сітки та алгоритмів
    data["simulation"]["show_grid"] = scene->showGrid;
    data["simulation"]["show_blocked_cells"] = scene->showBlockedCells;
    data["simulation"]["show_velocity_vector"] = scene->showVelocityVector;
    data["simulation"]["show_collision_point"] = scene->showCollisionPoint;
    data["simulation"]["cell_size"] = scene->pathfindingCellSize;
    data["simulation"]["autonomous_mode"] = scene->autonomousMode;
    data["simulation"]["selected_algorithm"] = scene->selectedAlgorithm;
    data["simulation"]["selected_costmap_source"] = scene->selectedCostmapSource;

    // Візуал траєкторії автопілота
    data["path"]["color"] = { scene->pathEntity->style.outlineColor.r, scene->pathEntity->style.outlineColor.g, scene->pathEntity->style.outlineColor.b, scene->pathEntity->style.outlineColor.a };
    data["path"]["line_width"] = scene->pathEntity->style.lineWidth;
    data["path"]["render_type"] = scene->pathEntity->renderType;

    // Візуал хрестика цілі
    data["target_view"]["color"] = { scene->targetPoint->style.outlineColor.r, scene->targetPoint->style.outlineColor.g, scene->targetPoint->style.outlineColor.b, scene->targetPoint->style.outlineColor.a };
    data["target_view"]["line_width"] = scene->targetPoint->style.lineWidth;

    std::ofstream file(fileName);
    if (file.is_open()) {
        file << data.dump(2);
    }
}

void ConfigLoader::loadAppSettings(const std::string& fileName, Scene* scene) {
    if (!scene || !scene->getRobot()) return;

    std::ifstream file(fileName);
    if (!file.is_open()) return;

    try {
        json data = json::parse(file);

        if (data.contains("lidar")) {
            auto lData = data["lidar"];
            auto& lidar = scene->getRobot()->getLidar();
            lidar.visible = lData.value("visible", true);
            lidar.setRayCount(lData.value("ray_count", 180));
            lidar.setMaxDistance(lData.value("max_distance", 300.0f));
            lidar.setFov(lData.value("fov", 3.14159f * 2.0f));
            lidar.rayColor = parseColor(lData["color"]);
        }

        if (data.contains("simulation")) {
            auto sData = data["simulation"];
            scene->showGrid = sData.value("show_grid", false);
            scene->showBlockedCells = sData.value("show_blocked_cells", false);
            scene->showVelocityVector = sData.value("show_velocity_vector", false);
            scene->showCollisionPoint = sData.value("show_collision_point", false);
            scene->pathfindingCellSize = sData.value("cell_size", 20.0f);
            scene->autonomousMode = sData.value("autonomous_mode", false);
            scene->selectedAlgorithm = sData.value("selected_algorithm", 0);
            scene->selectedCostmapSource = sData.value("selected_costmap_source", 0);

            // Синхронізуємо внутрішній прапорець лідара згідно з костмапом
            scene->useLidarForPathfinding = (scene->selectedCostmapSource == 1);
        }

        if (data.contains("path")) {
            auto pData = data["path"];
            scene->pathEntity->style.outlineColor = parseColor(pData["color"]);
            scene->pathEntity->style.lineWidth = pData.value("line_width", 2.5f);
            scene->pathEntity->renderType = pData.value("render_type", 0);
        }

        if (data.contains("target_view")) {
            auto tvData = data["target_view"];
            scene->targetPoint->style.outlineColor = parseColor(tvData["color"]);
            scene->targetPoint->style.lineWidth = tvData.value("line_width", 2.0f);
        }
    } catch (...) {
        std::cerr << "[ConfigLoader] Settings are corrupted, using defaults" << std::endl;
    }
}