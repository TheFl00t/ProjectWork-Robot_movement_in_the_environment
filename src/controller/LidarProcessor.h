#pragma once
#include "../model/Robot.h"
#include "../model/Environment.h"

class LidarProcessor {   
public:
    static void updateLidar(Robot& robot, const Environment& environment);
};