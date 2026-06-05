#pragma once

#include "../config.h"
#include "ShaderManager.h"
#include "Shader.h"

class Entity;
class Robot;
class Environment;
class CircleObstacle;
class RectObstacle;
class Line;
class Point;
class Mesh;

class Renderer {
private:
    ShaderManager* shaderManager;
    Shader* currentShader = nullptr;
    glm::mat4 projection;

    // Лінивий кеш для мешів: View сам створює та оновлює буфери OpenGL для об'єктів Моделі (щоб оптимізувати роботу)
    std::unordered_map<const Entity*, Mesh*> viewMeshCache;

    Renderer();
    void internalRender(const Entity* entity, Mesh* mesh);

public:
    static Renderer* getInstance();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void setShader(Shader* shader);
    void setProjection(const glm::mat4& proj);
    void applyProjectionToAllShaders();
    void clearCache();
    
    // Головний метод для рендеру (використовує поліморфізм для виклику потрібної функції малювання)
    void renderEntity(Entity* entity);

    // Окремі функції для відмальовування конкретних типів об'єктів нашої сцени
    void drawRobot(const Robot* robot);
    void drawEnvironment(const Environment* env);
    void drawCircleObstacle(const CircleObstacle* circle);
    void drawRectObstacle(const RectObstacle* rect);
    void drawLine(const Line* line);
    void drawPoint(const Point* point);
    
    // Очищення відеопам'яті під час видалення об'єкта з арени (щоб не було витоків пам'яті!)
    void freeEntityMesh(const Entity* entity);
};