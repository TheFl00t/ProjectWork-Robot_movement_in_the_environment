#pragma once

#include "../config.h"

class Mesh {
protected:
    GLuint VBO, VAO;
    
public:
    Mesh() : VBO(0), VAO(0) {};
    virtual ~Mesh() {};

    virtual void draw() = 0;
};