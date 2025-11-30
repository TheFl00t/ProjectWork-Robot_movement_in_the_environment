#version 330 core

layout(location=0) in vec2 vertPos;

uniform mat4 uModel;
uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * uModel * vec4(vertPos, 0.0, 1.0);
}