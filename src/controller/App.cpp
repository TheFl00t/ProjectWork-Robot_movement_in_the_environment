#include "App.h"
#include "ConfigLoader.h"
#include "InputManager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

App::~App() {
    delete scene;
    delete mapEditor;
    delete guiManager;
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
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
    window = glfwCreateWindow(winWidth, winHeight, "Robot Simulation & Map Editor", NULL, NULL);
    if (!window) { glfwTerminate(); return; }
    
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return;
    
    glViewport(0, 0, winWidth, winHeight);

    // Ініціалізація ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

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

    glPointSize(8.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window)) {
        float dt = computeDeltaTime();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        processInput();
        
        // Розподіл логіки оновлення між станами програми
        if (state == AppState::Simulation) {
            if (scene) scene->update(dt);
        } else if (state == AppState::Editor) {
            if (!ImGui::GetIO().WantCaptureMouse) {
                mapEditor->update(window, scene, getScreenToWorldMousePos());
            }
        }

        // Візуалізація світу
        if (scene) scene->render(renderer);

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

    // Робот керується лише в режимі симуляції
    if (state == AppState::Simulation && robot) {
        robot->direction = InputManager::getMovementDirection(window);

        if (InputManager::isKeyPressed(window, GLFW_KEY_R)) {
            robot->entityPos = robot->startPos;
            robot->direction = glm::vec2(0.f);
        }
    }

    if (InputManager::isKeyPressed(window, GLFW_KEY_TAB)) {
        if (guiManager) guiManager->resetGuiPos = true;
    }
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