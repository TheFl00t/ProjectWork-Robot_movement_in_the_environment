#include "CircleMesh.h"

#define DOUBLE_M_PI 6.283185307f

CircleMesh::CircleMesh() : radius(0.0f), vertex_count(0) {}

CircleMesh::CircleMesh(float r, unsigned vertCount)
    : radius(r), vertex_count(std::max(180u, vertCount))
{
    init();
}

CircleMesh::~CircleMesh() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void CircleMesh::init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    updateVertices(); // Заповнюємо буфер даними

    // Налаштування атрибутів
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
}

void CircleMesh::updateVertices() {
    std::vector<glm::vec2> vertices(vertex_count);
    for (unsigned int i = 0; i < vertex_count; i++) {
        float angle = DOUBLE_M_PI * i / vertex_count;
        vertices[i] = glm::vec2(cos(angle) * radius, sin(angle) * radius);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec2), vertices.data(), GL_DYNAMIC_DRAW);
}

void CircleMesh::setRadius(float r) {
    if (radius != r) { 
        radius = r;
        updateVertices();
    }
}

void CircleMesh::draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINE_LOOP, 0, vertex_count);
}