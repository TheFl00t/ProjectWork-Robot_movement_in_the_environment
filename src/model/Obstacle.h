#pragma once

#include <string>
#include "../json.hpp"
#include "Entity.h"

using json = nlohmann::json;
class Robot;

struct CollisionInfo {
    bool collided = false;
    glm::vec2 normal = glm::vec2(0.0f);       
    glm::vec2 contactPoint = glm::vec2(0.0f); 
    float depth = 0.0f;                       
};

class Obstacle : public Entity {
public:
    Obstacle(glm::vec2 pos) : Entity(pos) {};
    virtual ~Obstacle() {};

    virtual float intersectRay(const glm::vec2& rayStart, const glm::vec2& rayDir) const = 0;

    virtual CollisionInfo checkCollisionResult(Robot* robot) = 0;
    virtual std::string getTypeName() const = 0;
    virtual void serialize(json& j) const = 0;
};