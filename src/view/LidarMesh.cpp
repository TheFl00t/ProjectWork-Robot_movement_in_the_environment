#include "LidarMesh.h"

LidarMesh::LidarMesh() {
    init();
}

LidarMesh::~LidarMesh() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void LidarMesh::init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
}

void LidarMesh::updateLines(const glm::vec2& start, const std::vector<glm::vec2>& impacts) {
    std::vector<glm::vec2> vertices;
    vertices.reserve(impacts.size() * 2);

    for (const auto& ip : impacts) {
        vertices.push_back(start);
        vertices.push_back(ip);
    }

    vertexCount = static_cast<unsigned int>(vertices.size());

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_DYNAMIC_DRAW);
}

void LidarMesh::draw(GLenum topology) {
    if (vertexCount == 0) return;
    glBindVertexArray(VAO);
    glDrawArrays(topology, 0, vertexCount);
}