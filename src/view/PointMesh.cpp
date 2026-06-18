#include "PointMesh.h"

PointMesh::PointMesh() {
    init();
}

PointMesh::~PointMesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void PointMesh::init() {
    // Локальний центр (0,0), Renderer перемістить через uModel
    glm::vec2 vertex(0.0f, 0.0f);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2), &vertex, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
}

void PointMesh::draw(GLenum topology) {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, 1);
}