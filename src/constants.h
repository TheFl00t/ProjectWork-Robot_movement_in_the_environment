#pragma once

// === Robot ===
inline constexpr float ROBOT_RADIUS_MIN   = 5.0f;
inline constexpr float ROBOT_RADIUS_MAX   = 500.0f;
inline constexpr float ROBOT_VELOCITY_MIN = 0.0f;
inline constexpr float ROBOT_VELOCITY_MAX = 600.0f;

// === Obstacles ===
inline constexpr float CIRCLE_RADIUS_MIN  = 5.0f;
inline constexpr float CIRCLE_RADIUS_MAX  = 500.0f;
inline constexpr float RECT_SIZE_MIN      = 10.0f;
inline constexpr float RECT_SIZE_MAX      = 1000.0f;

// === Environment (Arena) ===
inline constexpr float ARENA_SIZE_MIN     = 100.0f;
inline constexpr float ARENA_SIZE_MAX     = 2048.0f;

// === Lidar ===
inline constexpr int   LIDAR_RAY_MIN      = 1;
inline constexpr int   LIDAR_RAY_MAX      = 720;
inline constexpr float LIDAR_DIST_MIN     = 0.0f;
inline constexpr float LIDAR_DIST_MAX     = 5000.0f;
inline constexpr float LIDAR_FOV_MIN      = 0.0f;
inline constexpr float LIDAR_FOV_MAX      = 6.283185f;

// === Navigation Grid ===
inline constexpr float GRID_CELL_SIZE = 20.0f;