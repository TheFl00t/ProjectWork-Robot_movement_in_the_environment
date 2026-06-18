#version 330 core

out vec4 screenColor;
uniform vec4 uColor;

void main() {
    screenColor = uColor;
}