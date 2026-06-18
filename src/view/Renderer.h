#pragma once

#include "../config.h"
#include "ShaderManager.h"
#include "Shader.h"
#include "LidarMesh.h"

class Entity;
class Robot;
class Environment;
class CircleObstacle;
class RectObstacle;
class Line;
class Point;
class PathEntity;
class Mesh;
class TargetPoint;

class Renderer {
private:
    ShaderManager* shaderManager;
    Shader* currentShader = nullptr;
    glm::mat4 projection;

    // Лінивий кеш для мешів: View сам створює та оновлює буфери OpenGL для об'єктів Моделі (щоб оптимізувати роботу)
    std::unordered_map<const Entity*, Mesh*> viewMeshCache;
    std::unordered_map<const Robot*, LidarMesh*> lidarMeshCache;
    std::unordered_map<const class PathEntity*, class PathMesh*> pathMeshCache;

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
    
    // Головний метод для рендеру
    void renderEntity(Entity* entity);

    // Окремі функції для відмальовування конкретних типів об'єктів сцени
    void drawRobot(const Robot* robot);
    void drawEnvironment(const Environment* env);
    void drawCircleObstacle(const CircleObstacle* circle);
    void drawRectObstacle(const RectObstacle* rect);
    void drawLine(const Line* line);
    void drawPoint(const Point* point);
    void drawLidar(const Robot* robot);
    void drawPath(const PathEntity* path);
    void drawGrid(const class Scene* scene);
    void drawTargetPoint(const TargetPoint* target);
    
    // Очищення відеопам'яті під час видалення об'єкта з арени
    void freeEntityMesh(const Entity* entity);
};