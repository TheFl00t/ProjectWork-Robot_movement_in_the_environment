cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME RobotSimulation)
project(${PROJECT_NAME} VERSION 1.0.0)

# Налаштування C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Додаємо папку cmake для пошуку модулів (FindGLFW3.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

# Пошук пакетів
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Збираємо файли ImGui
file(GLOB IMGUI_FILES
    dependencies/imgui/*.cpp
    dependencies/imgui/*.h
)

# Головний виконуваний файл
add_executable(${PROJECT_NAME}
    # Main & Config
    src/config.h
    src/main.cpp
    src/glad.c

    # ImGui
    ${IMGUI_FILES}

    # Model
    src/model/Entity.h
    src/model/Obstacle.cpp
    src/model/Environment.cpp
    src/model/Robot.cpp
    src/model/CircleObstacle.cpp
    src/model/RectObstacle.cpp
    src/model/Point.cpp
    src/model/Line.cpp

    # View
    src/view/Shader.cpp
    src/view/Renderer.cpp
    src/view/ShaderManager.cpp
    src/view/Mesh.h
    src/view/CircleMesh.cpp
    src/view/RectMesh.cpp
    src/view/PointMesh.cpp
    src/view/LineMesh.cpp

    # Controller
    src/controller/App.cpp
    src/controller/InputManager.cpp
    src/controller/Scene.cpp
    src/controller/ConfigLoader.cpp
)

# Якщо ми збираємо Release (фінальну версію), то приховуємо консоль
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # У Release режимі: додаємо властивість WIN32_EXECUTABLE, 
    # це додає прапор -mwindows, який прибирає чорне вікно.
    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)
endif()

# Шляхи для заголовків (Include Directories)
target_include_directories(${PROJECT_NAME} 
    PRIVATE dependencies
    PRIVATE dependencies/imgui
)

# Лінковка бібліотек
target_link_libraries(${PROJECT_NAME} 
    PRIVATE ${GLFW3_LIBRARY} OpenGL::GL
)

# 1. Копіюємо файл config.cfg поруч із .exe
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/config.cfg"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config.cfg"
)

# 2. Копіюємо всю папку src/shaders у папку збірки (зберігаючи структуру src/shaders)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/src/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/src/shaders"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/src/shaders"
)