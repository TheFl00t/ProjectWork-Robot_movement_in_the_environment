#include "LidarProcessor.h"
#include <cmath>
#include <algorithm>

void LidarProcessor::updateLidar(Robot& robot, const Environment& environment) {
    Lidar& lidar = robot.getLidar();
    auto& scanData = lidar.getScanDataRef();
    auto& impactPoints = lidar.getImpactPointsRef();

    glm::vec2 robotPos = robot.entityPos;

    if (glm::length(robot.direction) > 1e-5f) {
        robot.setAngle(std::atan2(robot.direction.y, robot.direction.x));
    }
    float robotAngle = robot.getAngle();

    int rayCount = lidar.getRayCount();
    float maxDist = lidar.getMaxDistance();
    float fov = lidar.getFov();

    bool isFullCircle = (std::abs(fov - 6.283185f) < 1e-3f);
    float angleStep = (rayCount > 1) ? (fov / (isFullCircle ? rayCount : (rayCount - 1))) : 0.0f;
    float startAngle = robotAngle - (fov / 2.0f);

    for (int i = 0; i < rayCount; ++i) {
        float currentAngle = startAngle + i * angleStep;
        glm::vec2 rayDir(std::cos(currentAngle), std::sin(currentAngle));
        float closestT = maxDist;

        // перебір усіх Obstacles
        for (const auto& obs : environment.getObstacles()) {
            float t = obs->intersectRay(robotPos, rayDir);
            if (t > 0.0f && t < closestT) {
                closestT = t;
            }
        }

        float tBorder = environment.intersectRay(robotPos, rayDir);
        if (tBorder > 0.0f && tBorder < closestT) {
            closestT = tBorder;
        }

        scanData[i] = closestT;
        impactPoints[i] = robotPos + rayDir * closestT;
    }
}