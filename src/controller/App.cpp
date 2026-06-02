#include "App.h"
#include "ConfigLoader.h"

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void App::run() {
    // Ініціалізація бібліотеки GLFW
    if (!glfwInit()) {
        std::cerr << "[App] Failed to initialize GLFW" << std::endl;
        return;
    }

    // --- НАЛАШТУВАННЯ ВІКНА ---

    // Зчитуємо розміри з конфігу
    int winWidth, winHeight;
    ConfigLoader::loadWindowSize("config.json", winWidth, winHeight);

    // Отримуємо розміри монітора
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);

    // Обмежуємо розміри (не менше 800x600 і не більше екрану)
    winWidth = std::clamp(winWidth, 800, mode->width);
    winHeight = std::clamp(winHeight, 600, mode->height);

    std::cout << "[App] Window size set to: " << winWidth << "x" << winHeight << std::endl;

    // Забороняємо зміну розміру вікна
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    // Створюємо вікно
    window = glfwCreateWindow(winWidth, winHeight, "Robot Simulation", NULL, NULL);
    if (!window) { 
        glfwTerminate(); 
        return; 
    }
    
    glfwMakeContextCurrent(window);
    
    // Ініціалізація GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[App] Failed to initialize GLAD" << std::endl;
        return;
    }
    
    glViewport(0, 0, winWidth, winHeight);

    // ===================================
    //          Налаштування ImGui
    // ===================================
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ===================================
    //          Ресурси та сцена
    // ===================================
    ShaderManager* shaderManager = ShaderManager::getInstance();
    // Завантажуємо шейдери
    shaderManager->loadShader("default", "shader.vert", "defaultShader.frag");

    Renderer* renderer = Renderer::getInstance();

    // Налаштовуємо матрицю проекції
    glm::mat4 proj = glm::ortho(0.0f, (float)winWidth, (float)winHeight, 0.0f, -1.0f, 1.0f);
    renderer->setProjection(proj);
    renderer->applyProjectionToAllShaders();

    // Завантажуємо сцену
    scene = ConfigLoader::loadScene("config.json", winWidth, winHeight);
    this->robot = scene->getRobot();

    // Налаштування OpenGL
    glPointSize(8.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ===================================
    //        Головний цикл симуляції
    // ===================================
    while (!glfwWindowShouldClose(window)) {
        float dt = computeDeltaTime();

        // Очищення буферу кольору
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Обробка введення
        processInput();
    
        // --- ImGui: Початок кадру ---
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        // --- Оновлення та рендеринг ---
        {
            scene->update(dt);
            scene->render(renderer);
        }

        // --- Побудова інтерфейсу ImGui ---
        {
            // Розрахунок позиції вікна налаштувань
            float windowW = 300.0f;
            float padding = 20.0f;
            float posX = ImGui::GetIO().DisplaySize.x - windowW - padding;
            float posY = padding;

            ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Appearing);
            ImGui::SetNextWindowSize(ImVec2(windowW, 450), ImGuiCond_Appearing);

            if (resetGuiPos) {
                ImGui::SetNextWindowPos(ImVec2(posX, posY), ImGuiCond_Always);
                resetGuiPos = false; 
            }

            ImGui::Begin("Simulation Control", NULL, ImGuiWindowFlags_NoResize);

            // 1. Інформація
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "[ Info ]");
            ImGui::Text("Performance: %.1f FPS", ImGui::GetIO().Framerate);
            if (robot) {
                ImGui::Text("Robot Pos: (%.1f, %.1f)", robot->entityPos.x, robot->entityPos.y);

                Point* p = scene->getDebugPoint();
                ImGui::Text("CollisionPoint Pos: (%.1f, %.1f)", p->entityPos.x, p->entityPos.y);
            }
            ImGui::Separator();

            // 2. Фізика
            if (robot) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "[ Physics ]");
                ImGui::SliderFloat("Velocity", &robot->velocity, 0.0f, 600.0f);

                if (ImGui::SliderFloat("Radius", &robot->radius, 5.0f, 100.0f)) {
                    if (auto mesh = dynamic_cast<CircleMesh*>(robot->getMesh())) {
                        mesh->setRadius(robot->radius);
                    }
                }
            }
            ImGui::Separator();

            // 3. Налагодження
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[ Debug ]");
            ImGui::Checkbox("Show Collision Point", &scene->showCollisionPoint);
            ImGui::Checkbox("Show Velocity Vector", &scene->showVelocityVector);
            ImGui::Separator();
            
            // ============================================================
            // НОВИЙ РОЗДІЛ: РЕДАКТОР КАРТ
            // ============================================================
            ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "[ Map Editor ]");
            
            // Перемикач режиму роботи програми
            if (ImGui::Checkbox("Enable Edit Mode", &isEditMode)) {
                if (isEditMode && robot) {
                    // Зупиняємо робота при вході в редактор, щоб він не уїхав
                    robot->direction = glm::vec2(0.f);
                }
            }

            if (isEditMode) {
                ImGui::Indent(); // Зсув вправо для візуального порядку

                // Вибір типу об'єкта
                ImGui::RadioButton("Circle", &selectedObstacleType, 0); ImGui::SameLine();
                ImGui::RadioButton("Rectangle", &selectedObstacleType, 1);

                // Налаштування геометричних розмірів залежно від вибору
                if (selectedObstacleType == 0) {
                    ImGui::SliderFloat("Radius##New", &newCircleRadius, 10.0f, 150.0f);
                } else {
                    ImGui::SliderFloat("Width##New", &newRectWidth, 10.0f, 300.0f);
                    ImGui::SliderFloat("Height##New", &newRectHeight, 10.0f, 300.0f);
                }

                // Налаштування стилю (матеріалу) для майбутнього об'єкта
                ImGui::Combo("Draw Mode##New", &newDrawMode, "Outline\0Fill\0FillAndOutline\0");
                ImGui::ColorEdit4("Fill Color##New", newFillColor);
                ImGui::ColorEdit4("Outline Color##New", newOutlineColor);
                ImGui::SliderFloat("Line Width##New", &newLineWidth, 1.0f, 5.0f);

                ImGui::Unindent();
            }
            ImGui::Separator();

            // 4. Система
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "[ System ]");
            
            if (ImGui::Button("Reset Position", ImVec2(150, 25))) {
                if (robot) {
                    robot->entityPos = robot->startPos;
                    robot->direction = glm::vec2(0.f);
                    scene->checkCollision(robot->entityPos); // Скинути точку зіткнення
                }
            }
            if (ImGui::Button("Reset Radius", ImVec2(150, 25))) {
                if (robot) {
                    robot->radius = robot->startRadius;
                    if (auto mesh = dynamic_cast<CircleMesh*>(robot->getMesh())) {
                        mesh->setRadius(robot->radius);
                    }
                }
            }
            if (ImGui::Button("Reset Velocity", ImVec2(150, 25))) {
                if (robot) robot->velocity = robot->startVelocity;
            }

            ImGui::Spacing();

            if (ImGui::Button("Open Config File", ImVec2(150, 25))) {   
                // Отримуємо повний шлях до файлу
                std::string fullPath = ConfigLoader::getConfigPath("config.json");
                
                std::cout << "[App] Opening config file..." << std::endl;
                // Запускаємо notepad.exe зі шляхом fullPath
                ShellExecuteA(NULL, "open", "notepad.exe", fullPath.c_str(), NULL, SW_SHOW);
            }

            if (ImGui::Button("Save Map", ImVec2(150, 25))) {
                if (scene) {
                    ConfigLoader::saveScene("config.json", scene, winWidth, winHeight);
                }
            }
            
            // Кнопка перезавантаження конфігурації
            if (ImGui::Button("Reload Config", ImVec2(150, 25))) {
                std::cout << "[App] Reloading scene..." << std::endl;
        
                // 1. Зчитуємо нові розміри вікна з файлу
                int newW, newH;
                ConfigLoader::loadWindowSize("config.json", newW, newH);

                // 2. Обмежуємо розміри
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                newW = std::clamp(newW, 800, mode->width);
                newH = std::clamp(newH, 600, mode->height);

                // 3. Якщо розмір змінився — застосовуємо зміни
                if (newW != winWidth || newH != winHeight) {
                    winWidth = newW;
                    winHeight = newH;

                    // Змінюємо розмір вікна GLFW
                    glfwSetWindowSize(window, winWidth, winHeight);
   
                    // Оновлюємо область рендерингу OpenGL
                    glViewport(0, 0, winWidth, winHeight);

                    // Оновлюємо матрицю проекції
                    glm::mat4 newProj = glm::ortho(0.0f, (float)winWidth, (float)winHeight, 0.0f, -1.0f, 1.0f);
                    renderer->setProjection(newProj);
                    renderer->applyProjectionToAllShaders();
                    
                    std::cout << "[App] Window resized to: " << winWidth << "x" << winHeight << std::endl;
                }

                // 4. Спочатку завантажуємо нову сцену в тимчасову змінну
                Scene* newScene = ConfigLoader::loadScene("config.json", winWidth, winHeight);
                
                // 5. Перевіряємо, чи не повернула функція нульовий вказівник через критичну помилку
                if (newScene == nullptr) {
                    std::cerr << "[App] Reload cancelled. Configuration file failed to load." << std::endl;
                } else {
                    // Тільки тепер, коли ми впевнені, що нова сцена готова, видаляємо стару
                    delete scene;
                    scene = newScene;
                    this->robot = scene->getRobot();
                    std::cout << "[App] Scene successfully updated!" << std::endl;
                }
            }

            ImGui::End();
        }

        // --- ImGui та OpenGL: Рендеринг ---
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    // --- КІНЕЦЬ ЦИКЛУ ---

    // Очищення ресурсів
    delete scene;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void App::processInput() {
    glm::vec2 dir = InputManager::getMovementDirection(window);
    if (robot) robot->direction = dir;

    if (InputManager::isKeyPressed(window, GLFW_KEY_R)) {
        if (robot) {
            robot->entityPos = robot->startPos;
            robot->direction = glm::vec2(0.f);
        }
    }

    if (InputManager::isKeyPressed(window, GLFW_KEY_TAB)) {
        resetGuiPos = true;
    }

    // --- ОБРАБОТКА МЫШИ ДЛЯ РЕДАКТОРА КАРТ ---
    
    // --- ОБРАБОТКА МЫШИ ДЛЯ РЕДАКТОРА КАРТ ---
    if (ImGui::GetIO().WantCaptureMouse) {
        return; 
    }

    static bool wasLeftPressed = false;
    int leftMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    if (leftMouseState == GLFW_PRESS && !wasLeftPressed) {
        wasLeftPressed = true;

        // Клік обробляється ТІЛЬКИ якщо активовано режим редактора
        if (isEditMode && scene) {
            glm::vec2 worldMousePos = getScreenToWorldMousePos();
            Obstacle* newObstacle = nullptr;

            // 1. Створюємо об'єкт потрібної геометрії
            if (selectedObstacleType == 0) {
                newObstacle = new CircleObstacle(worldMousePos, newCircleRadius);
            } else {
                newObstacle = new RectObstacle(worldMousePos, newRectWidth, newRectHeight);
            }

            if (newObstacle) {
                // 2. Переносимо налаштування матеріалу з UI в об'єкт
                newObstacle->style.mode = static_cast<DrawMode>(newDrawMode);
                newObstacle->style.lineWidth = newLineWidth;
                newObstacle->style.fillColor = glm::vec4(newFillColor[0], newFillColor[1], newFillColor[2], newFillColor[3]);
                newObstacle->style.outlineColor = glm::vec4(newOutlineColor[0], newOutlineColor[1], newOutlineColor[2], newOutlineColor[3]);

                // 3. Додаємо об'єкт на карту в пам'ять
                scene->getEnvironmentPointer()->addObstacle(newObstacle);
                std::cout << "[Editor] Spawned new obstacle at: (" << worldMousePos.x << ", " << worldMousePos.y << ")" << std::endl;
            }
        }
    } 
    else if (leftMouseState == GLFW_RELEASE) {
        wasLeftPressed = false;
    }
}

glm::vec2 App::getScreenToWorldMousePos() {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    // Зараз у нас 1 піксель = 1 юніт світу, тому повертаємо як є
    return glm::vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
}

float App::computeDeltaTime() {
    static float lastTime = glfwGetTime();
    float currentTime = glfwGetTime();
    float dt = currentTime - lastTime;
    lastTime = currentTime;
    
    if (dt > 0.1f) {
        dt = 0.1f;
    }

    return dt;
}