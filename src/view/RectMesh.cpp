#include "RectMesh.h"

RectMesh::RectMesh()
    : width(0.0f), height(0.0f), vertex_count(0) {}

RectMesh::RectMesh(float width, float height)
    : width(width), height(height)
{
    init();
}

RectMesh::~RectMesh() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void RectMesh::init() {
    std::vector<glm::vec2> vertices = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(width, 0.0f),
        glm::vec2(width, height),
        glm::vec2(0.0f, height)
    };
    vertex_count = vertices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
}

void RectMesh::draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, vertex_count);
}