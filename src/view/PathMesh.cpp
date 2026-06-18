#include "PathMesh.h"

PathMesh::PathMesh() {
    init();
}

PathMesh::~PathMesh() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void PathMesh::init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
}

void PathMesh::updatePath(const std::vector<glm::vec2>& points) {
    vertexCount = static_cast<unsigned int>(points.size());
    if (vertexCount == 0) return;

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec2), points.data(), GL_DYNAMIC_DRAW);
}

void PathMesh::draw(GLenum topology) {
    if (vertexCount == 0) return;
    glBindVertexArray(VAO);
    glDrawArrays(topology, 0, vertexCount);
}