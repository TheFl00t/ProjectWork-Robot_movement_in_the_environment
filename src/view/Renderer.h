#pragma once

#include "../config.h"
#include "../model/Robot.h"
#include "../model/Obstacle.h"
#include "../model/Environment.h"
#include "../model/Point.h"
#include "../model/Line.h"
#include "ShaderManager.h"
#include "Shader.h"

class Renderer {
private:
    ShaderManager* shaderManager;
    Shader* currentShader = nullptr;
    glm::mat4 projection;

    Renderer();

public:
    static Renderer* getInstance();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void setShader(Shader* shader);
    void setProjection(const glm::mat4& proj);
    
    // Оновлює матрицю проекції у всіх шейдерах (при зміні розміру вікна)
    void applyProjectionToAllShaders();

    // Головний метод рендеру сцени
    void render(Robot* robot, Environment* env);
    
    // Спеціалізовані методи
    void renderEnvironment(Environment* env);
    void renderWalls(Environment* env);
    void renderRobot(Robot* robot);
    void renderObstacle(Obstacle* obstacle);
    void renderPoint(Point* point);
    void renderLine(Line* line);
};