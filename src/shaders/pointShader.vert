#version 330 core

layout(location=0) in vec4 vertCol; // Колір

out vec4 color;

uniform mat4 uModel;
uniform mat4 uProjection;

void main() {
    gl_Position = uProjection * uModel * vec4(0.0, 0.0, 0.0, 1.0);
    
    gl_PointSize = 8.0;
    color = vertCol;
}