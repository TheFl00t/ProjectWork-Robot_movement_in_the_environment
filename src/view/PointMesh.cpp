#include "PointMesh.h"

PointMesh::PointMesh() {
    init();
}

PointMesh::~PointMesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void PointMesh::init() {
    // Точка знаходиться в локальному центрі. Renderer сам змістить її куди потрібно
    glm::vec2 vertex(0.0f, 0.0f);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2), &vertex, GL_STATIC_DRAW);

    // Передаємо 2 флоати (координати X та Y) в location = 0 для універсального shader.vert
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
}

void PointMesh::draw(GLenum topology) {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, 1);
}