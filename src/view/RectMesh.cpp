#include "RectMesh.h"

RectMesh::RectMesh()
    : width(0.0f), height(0.0f), vertex_count(0), centered(true) {}

RectMesh::RectMesh(float width, float height, bool centered)
    : width(width), height(height), centered(centered)
{
    init();
}

RectMesh::~RectMesh() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void RectMesh::init() {
    std::vector<glm::vec2> vertices;

    if (centered) {
        // Вариант для РЕДАКТОРА: координаты строятся симметрично вокруг центра
        float hw = width * 0.5f;
        float hh = height * 0.5f;
        vertices = {
            glm::vec2(-hw, -hh), // Левый верхний угол
            glm::vec2( hw, -hh), // Правый верхний угол
            glm::vec2( hw,  hh), // Правый нижний угол
            glm::vec2(-hw,  hh)  // Левый нижний угол
        };
    } else {
        // Вариант для АРЕНЫ: координаты строятся от верхнего левого угла
        vertices = {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(width, 0.0f),
            glm::vec2(width, height),
            glm::vec2(0.0f, height)
        };
    }
    
    vertex_count = vertices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
}

void RectMesh::draw(GLenum topology) {
    glBindVertexArray(VAO);
    glDrawArrays(topology, 0, vertex_count);
}

void RectMesh::updateDimensions(float newWidth, float newHeight) {
    width = newWidth;
    height = newHeight;

    std::vector<glm::vec2> vertices;
    if (centered) {
        float hw = width * 0.5f;
        float hh = height * 0.5f;
        vertices = {
            glm::vec2(-hw, -hh), glm::vec2( hw, -hh),
            glm::vec2( hw,  hh), glm::vec2(-hw,  hh)
        };
    } else {
        vertices = {
            glm::vec2(0.0f, 0.0f), glm::vec2(width, 0.0f),
            glm::vec2(width, height), glm::vec2(0.0f, height)
        };
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);
}