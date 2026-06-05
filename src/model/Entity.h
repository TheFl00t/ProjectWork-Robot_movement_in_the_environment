#pragma once

#include "../config.h"

class Renderer;

enum class DrawMode {
    Outline,       
    Fill,          
    FillAndOutline 
};

struct RenderStyle {
    DrawMode mode = DrawMode::Outline;
    glm::vec4 fillColor = glm::vec4(1.0f);      
    glm::vec4 outlineColor = glm::vec4(1.0f);   
    float lineWidth = 1.0f;                     
};

class Entity {
public:
    glm::vec2 entityPos;
    RenderStyle style; 
    
    Entity(glm::vec2 pos = glm::vec2(0.0f, 0.0f)) : entityPos(pos) {};
    virtual ~Entity() {};

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    virtual void update(float dt) = 0;
    virtual bool containsPoint(glm::vec2 point) { return false; }
    virtual bool getBounds(glm::vec2& outMin, glm::vec2& outMax) const { return false; }
    virtual void resizeByGizmo(const glm::vec2& mousePos) {}
    virtual void drawVisitor(class Renderer* renderer) = 0;
};