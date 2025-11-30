#include "PointMesh.h"

PointMesh::PointMesh()
    : color(glm::vec4(1.f, 0.f, 0.f, 1.f)) 
{
    init();
}

PointMesh::PointMesh(glm::vec4 col)
    : color(col)
{
    init();
}

PointMesh::~PointMesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void PointMesh::init() {
    vertex_count = 1;
    std::vector<float> vertex = {color.r, color.g, color.b, color.a};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(float), vertex.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertex.size() * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

// Функцiя для швидкого перерахування та оновлення колiру у VBO
void PointMesh::updateVertices() {
    std::vector<float> vertex = {color.r, color.g, color.b, color.a};

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertex.size() * sizeof(float), vertex.data());
}

void PointMesh::setColor(glm::vec4 col) {
    color = col;
    updateVertices();
}

void PointMesh::draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, 1);
}