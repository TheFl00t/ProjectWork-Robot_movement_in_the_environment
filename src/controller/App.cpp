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
    ConfigLoader::loadWindowSize("config.cfg", winWidth, winHeight);

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
    //            ImGui Setup
    // ===================================
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ===================================
    //         Resources & Scene
    // ===================================
    ShaderManager* shaderManager = ShaderManager::getInstance();
    // Завантажуємо шейдери
    shaderManager->loadShader("obstacle", "shader.vert", "shader.frag");
    shaderManager->loadShader("robot", "robotShader.vert", "robotShader.frag");
    shaderManager->loadShader("walls", "wallsShader.vert", "wallsShader.frag");
    shaderManager->loadShader("point", "pointShader.vert", "pointShader.frag");

    Renderer* renderer = Renderer::getInstance();

    // Налаштовуємо матрицю проекції
    glm::mat4 proj = glm::ortho(0.0f, (float)winWidth, (float)winHeight, 0.0f, -1.0f, 1.0f);
    renderer->setProjection(proj);
    renderer->applyProjectionToAllShaders();

    // Завантажуємо сцену
    Scene* scene = ConfigLoader::loadScene("config.cfg", winWidth, winHeight);
    this->robot = scene->getRobot();

    // Налаштування OpenGL
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ===================================
    //             Main Loop
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

        // --- Оновлення та Рендер ---
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
            
            // 4. Система
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "[ System ]");
            
            if (ImGui::Button("Reset Position", ImVec2(150, 25))) {
                if (robot) {
                    robot->entityPos = robot->startPos;
                    robot->direction = glm::vec2(0.f);
                    scene->checkCollision(robot->entityPos); // Скинути точку колізії
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
                std::string fullPath = ConfigLoader::getConfigPath("config.cfg");
                
                // Запускаємо notepad.exe зi шляхом fullPath
                ShellExecuteA(NULL, "open", "notepad.exe", fullPath.c_str(), NULL, SW_SHOW);
            }
            
            // Кнопка перезавантаження конфігурації
            if (ImGui::Button("Reload Config", ImVec2(150, 25))) {
                std::cout << "[App] Reloading scene..." << std::endl;
        
                // 1. Зчитуємо нові розміри вікна з файлу
                int newW, newH;
                ConfigLoader::loadWindowSize("config.cfg", newW, newH);

                // 2. Обмежуємо розміри
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                newW = std::clamp(newW, 800, mode->width);
                newH = std::clamp(newH, 600, mode->height);

                // 3. Якщо розмір змінився - застосовуємо зміни
                if (newW != winWidth || newH != winHeight) {
                    winWidth = newW;
                    winHeight = newH;

                    // Змінюємо розмір вікна GLFW
                    glfwSetWindowSize(window, winWidth, winHeight);
   
                    // Оновлюємо область рендеру OpenGL
                    glViewport(0, 0, winWidth, winHeight);

                    // Оновлюємо матрицю проекції
                    glm::mat4 newProj = glm::ortho(0.0f, (float)winWidth, (float)winHeight, 0.0f, -1.0f, 1.0f);
                    renderer->setProjection(newProj);
                    renderer->applyProjectionToAllShaders();
                    
                    std::cout << "[App] Window resized to: " << winWidth << "x" << winHeight << std::endl;
                }

                // 4. Видаляємо стару сцену
                delete scene;
                
                // 5. Завантажуємо нову сцену (з новими winWidth/winHeight)
                scene = ConfigLoader::loadScene("config.cfg", winWidth, winHeight);
                
                // Оновлюємо посилання на робота
                this->robot = scene->getRobot();
            }

            ImGui::End();
        }

        // --- ImGui & OpenGL: Рендер ---
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
}

float App::computeDeltaTime() {
    static float lastTime = glfwGetTime();
    float currentTime = glfwGetTime();
    float dt = currentTime - lastTime;
    lastTime = currentTime;
    return dt;
}