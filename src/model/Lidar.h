#pragma once

#include "../config.h"

class Lidar {
private:
    int rayCount;
    float maxDistance;
    float fov;

    std::vector<float> scanData;
    std::vector<glm::vec2> impactPoints;

public:
    glm::vec4 rayColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.3f);
    bool visible = true;
    
    Lidar(int rayCount = 180, float maxDistance = 300.0f, float fov = 3.14159f * 2);
    
    void resizeBuffers();
    
    int getRayCount() const { return rayCount; }
    void setRayCount(int count);
    
    float getMaxDistance() const { return maxDistance; }
    void setMaxDistance(float dist);

    float getFov() const { return fov; }
    void setFov(float fov);
    
    const std::vector<float>& getScanData() const { return scanData; }
    const std::vector<glm::vec2>& getImpactPoints() const { return impactPoints; }
    
    std::vector<float>& getScanDataRef() { return scanData; }
    std::vector<glm::vec2>& getImpactPointsRef() { return impactPoints; }
};