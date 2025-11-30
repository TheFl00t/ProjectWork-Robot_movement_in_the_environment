#pragma once

#include "../config.h"
#include "Shader.h"

class ShaderManager {
private:
    ShaderManager() {}
    ~ShaderManager();

protected:
    std::unordered_map<std::string, Shader*> shaders;

public:
    static ShaderManager* getInstance();

    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    const std::unordered_map<std::string, Shader*>& getAllShaders() const { return shaders; }
    
    // Завантажує новий шейдер або повертає існуючий
    Shader* loadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    
    // Отримує вже завантажений шейдер
    Shader* getShader(const std::string& name) const;
};