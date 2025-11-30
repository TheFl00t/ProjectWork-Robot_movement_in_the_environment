#include "LineMesh.h"

LineMesh::LineMesh() {
    init();
}

LineMesh::~LineMesh() {
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void LineMesh::init() {
    // Початкові координати (нульова довжина)
    std::vector<glm::vec2> vertices = {
        glm::vec2(0.0f, 0.0f),
        glm::vec2(0.0f, 0.0f)
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec2), vertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);
}

void LineMesh::updateLine(glm::vec2 start, glm::vec2 end) {
    std::vector<glm::vec2> vertices = { start, end };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 2 * sizeof(glm::vec2), vertices.data());
}

void LineMesh::draw() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 2);
}