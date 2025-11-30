#pragma once

#include "../config.h"

class Shader {
public:
    GLuint ID; // ID програми в OpenGL

    Shader(std::string vertex_shader_fileName, std::string fragment_shader_fileName);
    ~Shader();

    // Активувати цей шейдер для малювання
    void use() const;

    // Методи для передачі даних (Uniforms) у шейдер
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    // Допоміжні методи
    std::string getFilePath(const std::string& fileName);
    GLuint make_module(const std::string& filepath, GLuint module_type);
    GLuint make_shader(const std::string& vertex_filepath, const std::string& fragment_filepath);
};