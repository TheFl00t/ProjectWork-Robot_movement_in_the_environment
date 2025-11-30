#include "ConfigLoader.h"

namespace fs = std::filesystem;

// Внутрішні типи для парсингу
enum ConfigType { TYPE_UNKNOWN, TYPE_WINDOW, TYPE_ENV, TYPE_ROBOT, TYPE_CIRCLE, TYPE_RECT };
enum AlignMode { ALIGN_CUSTOM, ALIGN_TOP_LEFT, ALIGN_TOP_RIGHT, ALIGN_BOTTOM_LEFT, ALIGN_BOTTOM_RIGHT, ALIGN_CENTER };

// Допоміжна функція: String -> ConfigType
ConfigType stringToConfigType(const std::string& str) {
    if (str == "WINDOW") return TYPE_WINDOW;
    if (str == "ENV")    return TYPE_ENV;
    if (str == "ROBOT")  return TYPE_ROBOT;
    if (str == "CIRCLE") return TYPE_CIRCLE;
    if (str == "RECT")   return TYPE_RECT;
    return TYPE_UNKNOWN;
}

// Допоміжна функція: String -> AlignMode
AlignMode stringToAlignMode(const std::string& str) {
    if (str == "CENTER")       return ALIGN_CENTER;
    if (str == "TOP_LEFT")     return ALIGN_TOP_LEFT;
    if (str == "TOP_RIGHT")    return ALIGN_TOP_RIGHT;
    if (str == "BOTTOM_LEFT")  return ALIGN_BOTTOM_LEFT;
    if (str == "BOTTOM_RIGHT") return ALIGN_BOTTOM_RIGHT;
    return ALIGN_CUSTOM;
}

std::string ConfigLoader::getConfigPath(const std::string& fileName) {
    fs::path currentPath = fs::current_path();
    
    // Список шляхів для пошуку
    std::vector<fs::path> searchPaths = {
        currentPath / fileName,
        currentPath / "../../" / fileName,
        currentPath / "src" / fileName
    };

    for (const auto& path : searchPaths) {
        if (fs::exists(path)) {
            std::cout << "[ConfigLoader] Found config at: " << path.string() << std::endl;
            return path.string();
        }
    }

    std::cerr << "[ConfigLoader] ERROR: Config file '" << fileName << "' not found!" << std::endl;
    return ""; // Обробимо далi
}

void ConfigLoader::loadWindowSize(const std::string& fileName, int& width, int& height) {
    width = 800; height = 600; // Default
    std::string fullPath = getConfigPath(fileName);
    if (fullPath.empty()) return;

    std::ifstream file(fullPath);
    if (!file.is_open()) return;

    bool found = false;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string typeStr; 
        ss >> typeStr;

        if (stringToConfigType(typeStr) == TYPE_WINDOW) {
            ss >> width >> height;
            found = true;
            break;
        }
    }
    width = fabs(width);
    height = fabs(height);
    file.close();
}

Scene* ConfigLoader::loadScene(const std::string& fileName, int winWidth, int winHeight) {
    std::string fullPath = getConfigPath(fileName);
    std::ifstream file;
    if (!fullPath.empty()) file.open(fullPath);

    // Default
    float envW = 800.0f, envH = 600.0f;
    glm::vec2 envPos(0.0f, 0.0f);

    if (!file.is_open()) {
        std::cerr << "[ConfigLoader] Loading DEFAULT scene." << std::endl;
        Environment* env = new Environment(envPos, envW, envH);
        Robot* rob = new Robot(glm::vec2(400,300), 30, 100);
        return new Scene(rob, env);
    }

    Environment* env = nullptr;
    Robot* robot = nullptr;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue; 
        std::stringstream ss(line);
        std::string typeStr; 
        ss >> typeStr;

        switch (stringToConfigType(typeStr)) {
            case TYPE_ENV: {
                ss >> envW >> envH;
                std::string alignStr;
                AlignMode mode = ALIGN_TOP_LEFT;
                
                if (ss >> alignStr) 
                    mode = stringToAlignMode(alignStr);

                envW = fabs(envW);
                envH = fabs(envH);
                // Розрахунок позиції середовища
                switch (mode) {
                    case ALIGN_CENTER:
                        envPos.x = (winWidth - envW) / 2.0f;
                        envPos.y = (winHeight - envH) / 2.0f;
                        break;
                    case ALIGN_TOP_RIGHT:
                        envPos.x = winWidth - envW; envPos.y = 0.0f; break;
                    case ALIGN_BOTTOM_LEFT:
                        envPos.x = 0.0f; envPos.y = winHeight - envH; break;
                    case ALIGN_BOTTOM_RIGHT:
                        envPos.x = winWidth - envW; envPos.y = winHeight - envH; break;
                    default: envPos = glm::vec2(0,0); break;
                }
                
                // Перевірка, щоб середовище не виходило за вікно
                envPos.x = std::clamp(envPos.x, 0.0f, (float)winWidth - envW);
                envPos.y = std::clamp(envPos.y, 0.0f, (float)winHeight - envH);

                env = new Environment(envPos, envW, envH);
                break;
            }
            case TYPE_ROBOT: {
                float x, y, r, v;
                ss >> x >> y >> r >> v;
                robot = new Robot(glm::vec2(x, y), std::clamp(r, 5.0f, 100.f), v);
                break;
            }
            case TYPE_CIRCLE: {
                float x, y, r;
                ss >> x >> y >> r;
                if (env) env->addObstacle(new CircleObstacle(glm::vec2(x, y), r));
                break;
            }
            case TYPE_RECT: {
                float x, y, w, h;
                ss >> x >> y >> w >> h;
                if (env) env->addObstacle(new RectObstacle(glm::vec2(x, y), w, h));
                break;
            }
            default: break;
        }
    }
    file.close();
    

    // Створення default об'єктів, якщо їх не було в файлі
    if (!env) env = new Environment(envPos, envW, envH);
    if (!robot) robot = new Robot(glm::vec2(100,100), 20, 100);

    // Перевіряємо, чи не з'явився робот у стіні
    float minX = envPos.x + robot->radius;
    float maxX = envPos.x + envW - robot->radius;
    float minY = envPos.y + robot->radius;
    float maxY = envPos.y + envH - robot->radius;

    if (minX > maxX) maxX = minX; 
    if (minY > maxY) maxY = minY;

    glm::vec2 oldPos = robot->entityPos;
    robot->entityPos.x = std::clamp(robot->entityPos.x, minX, maxX);
    robot->entityPos.y = std::clamp(robot->entityPos.y, minY, maxY);

    if (robot->entityPos != oldPos) {
        std::cout << "[ConfigLoader] Warning: Robot moved inside environment bounds." << std::endl;
        robot->startPos = robot->entityPos; // Оновлюємо стартову позицію
    }

    return new Scene(robot, env);
}