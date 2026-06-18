#include "App.h"
#include "ConfigLoader.h"
#include "InputManager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <dwmapi.h>

App::~App() {
    if (scene) {
        ConfigLoader::saveAppSettings("settings.json", scene);
    }

    delete scene;
    delete mapEditor;
    delete guiManager;
    
    if (imguiInitialized) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    if (window) {
        glfwDestroyWindow(window);
    }
    
    glfwTerminate();
}

void App::run() {
    if (!glfwInit()) return;
    std::string configPath = ConfigLoader::getConfigPath("config.json", false);
    ConfigLoader::loadWindowSize(configPath, winWidth, winHeight);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    winWidth = std::clamp(winWidth, 800, mode->width);
    winHeight = std::clamp(winHeight, 600, mode->height);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    if (winWidth == mode->width && winHeight == mode->height) {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }

    window = glfwCreateWindow(winWidth, winHeight, "Robot Simulation", NULL, NULL);
    if (!window) { glfwTerminate(); return; }

    #define DWMWA_CAPTION_COLOR 35
    #define DWMWA_TEXT_COLOR 36
    HWND hwnd = glfwGetWin32Window(window);
    COLORREF titleBarColor = RGB(24, 24, 24);
    COLORREF textColor = RGB(255, 255, 255);
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &titleBarColor, sizeof(titleBarColor));
    DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &textColor, sizeof(textColor));
    
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return;
    
    glViewport(0, 0, winWidth, winHeight);

    // Ініціалізація ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg]  = ImVec4(0.094f, 0.094f, 0.094f, 0.7f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.094f, 0.094f, 0.094f, 1.0f);
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    imguiInitialized = true;

    // Завантаження ресурсів
    ShaderManager::getInstance()->loadShader("default", "shader.vert", "defaultShader.frag");
    Renderer* renderer = Renderer::getInstance();
    
    glm::mat4 proj = glm::ortho(0.0f, (float)winWidth, (float)winHeight, 0.0f, -1.0f, 1.0f);
    renderer->setProjection(proj);
    renderer->applyProjectionToAllShaders();

    // Створення підсистем
    scene = ConfigLoader::loadScene(configPath, winWidth, winHeight);
    mapEditor = new MapEditor();
    guiManager = new GuiManager();

    ConfigLoader::loadAppSettings("settings.json", scene);

    glPointSize(8.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        float dt = computeDeltaTime();

        glClearColor(0.157f, 0.157f, 0.157f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (state == AppState::Simulation && lastState == AppState::Editor) {
            if (scene && scene->autonomousMode && scene->targetPoint->active) {
                scene->requestPathUpdate();
            }
        }
        lastState = state;
        processInput();
        
        // Розподіл логіки оновлення між станами програми
        if (state == AppState::Simulation) {
            if (scene) scene->update(dt);
        } else if (state == AppState::Editor) {
            if (!ImGui::GetIO().WantCaptureMouse && !scene->targetPoint->isDragging) {
                mapEditor->update(window, scene, getScreenToWorldMousePos());
            }
            if (scene) scene->updateEditor();
        }

        // Візуалізація світу
        if (scene) scene->render(renderer, state);

        // Візуалізація інтерфейсу користувача
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        guiManager->render(window, scene, mapEditor, state, winWidth, winHeight);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App::processInput() {
    if (!scene) return;
    Robot* robot = scene->getRobot();

    // Drag-and-Drop лівою кнопкою миші у будь-якому режимі додатка
    int leftMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    glm::vec2 mouseWorld = getScreenToWorldMousePos();

    if (leftMouseState == GLFW_PRESS) {
        // Перевіряємо клік по хрестику цілі, якщо захоплення ще не відбулося
        if (!scene->targetPoint->isDragging && scene->targetPoint->containsPoint(mouseWorld)) {
            scene->targetPoint->isDragging = true;
            scene->targetPoint->isMovingTo = false;
        }
        // Оновлюємо координати цілі під час утримання миші
        if (scene->targetPoint->isDragging) {
            scene->targetPoint->entityPos = mouseWorld;
            scene->requestPathUpdate();
        }
    } else if (leftMouseState == GLFW_RELEASE) {
        scene->targetPoint->isDragging = false;
    }

    // Робот керується лише в режимі симуляції
    if (state == AppState::Simulation && robot) {
        static bool wasSpacePressed = false;
        bool isSpaceDown = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

        // Перевіряємо, що користувач не пише текст в інтерфейсі прямо зараз
        if (!ImGui::GetIO().WantTextInput) {
            if (isSpaceDown && !wasSpacePressed) {
                // Перевіряємо, чи затиснутий лівий або правий Ctrl
                bool isCtrlDown = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || 
                                   glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
                
                if (isCtrlDown) {
                    // Ctrl + Space -> Глобальна пауза симуляції (працює і в ручному, і в авто-режимі)
                    scene->isPaused = !scene->isPaused;
                } else if (scene->autonomousMode && scene->targetPoint->active) {
                    // Space -> Старт / Стоп фізичного руху автопілота робота
                    scene->targetPoint->isMovingTo = !scene->targetPoint->isMovingTo;
                    if (scene->targetPoint->isMovingTo) {
                        scene->requestPathUpdate(); // Оновлюємо карту при старті руху
                    }
                }
            }
        }
        wasSpacePressed = isSpaceDown;

        if (!scene->autonomousMode) {
            bool isCtrlDown = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || 
                               glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

            if (scene->isPaused || isCtrlDown) {
                robot->direction = glm::vec2(0.0f); // Скидаємо рух при паузі або хоткеях
            } else {
                robot->direction = InputManager::getMovementDirection(window);
            }
        } else {
            int rightMouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
            if (rightMouseState == GLFW_PRESS) {
                scene->setTarget(getScreenToWorldMousePos());
            }
        }

        if (InputManager::isKeyPressed(window, GLFW_KEY_R)) {
            robot->entityPos = robot->startPos;
            robot->direction = glm::vec2(0.f);
            scene->targetPoint->active = false;
            scene->targetPoint->isMovingTo = false;
            scene->autonomousMode = false;
        }
    }

    if (!ImGui::GetIO().WantTextInput) {
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            state = AppState::Simulation;
        } else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            state = AppState::Editor;
            if (scene && scene->getRobot()) {
                scene->getRobot()->direction = glm::vec2(0.f);
            }
        }
    }

    // Trigger для одиночного натискання TAB
    static bool wasTabPressed = false;
    bool isTabDown = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
    if (isTabDown && !wasTabPressed) {
        if (guiManager && !ImGui::GetIO().WantTextInput) {
            guiManager->showUi = !guiManager->showUi; // Перемикаємо видимість вікон
        }
    }
    wasTabPressed = isTabDown;
}

glm::vec2 App::getScreenToWorldMousePos() {
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    return glm::vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
}

float App::computeDeltaTime() {
    static float lastTime = glfwGetTime();
    float currentTime = glfwGetTime();
    float dt = currentTime - lastTime;
    lastTime = currentTime;
    return (dt > 0.1f) ? 0.1f : dt;
}