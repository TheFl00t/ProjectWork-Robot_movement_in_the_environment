#include "Lidar.h"
#include "../constants.h"
#include <algorithm>

Lidar::Lidar(int rayCount, float maxDistance, float fov) {
    this->rayCount    = std::clamp(rayCount,    LIDAR_RAY_MIN,  LIDAR_RAY_MAX);
    this->maxDistance = std::clamp(maxDistance, LIDAR_DIST_MIN, LIDAR_DIST_MAX);
    this->fov         = std::clamp(fov,         LIDAR_FOV_MIN,  LIDAR_FOV_MAX);
    this->resizeBuffers();
}

void Lidar::setRayCount(int count) {
    rayCount = std::clamp(count, LIDAR_RAY_MIN, LIDAR_RAY_MAX);
    resizeBuffers();
}

void Lidar::setMaxDistance(float dist) {
    maxDistance = std::clamp(dist, LIDAR_DIST_MIN, LIDAR_DIST_MAX);
}

void Lidar::setFov(float fov) {
    this->fov = std::clamp(fov, LIDAR_FOV_MIN, LIDAR_FOV_MAX);
}

void Lidar::resizeBuffers() {
    this->scanData.resize(this->rayCount, this->maxDistance);
    this->impactPoints.resize(this->rayCount, glm::vec2(0.0f));
}