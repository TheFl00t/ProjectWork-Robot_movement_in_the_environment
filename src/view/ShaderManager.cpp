#include "ShaderManager.h"

ShaderManager* ShaderManager::getInstance() {
    static ShaderManager instance;
    return &instance;
}

ShaderManager::~ShaderManager() {
    for (auto& [name, shader] : shaders) {
        delete shader;
    }
    shaders.clear();
}

Shader* ShaderManager::loadShader(
    const std::string& name,
    const std::string& vertex_shader_fileName,
    const std::string& fragment_shader_fileName
) {
    if (shaders.find(name) != shaders.end())
        return shaders[name];
    
    Shader* s = new Shader(vertex_shader_fileName, fragment_shader_fileName);
    shaders[name] = s;
    return s;
}

Shader* ShaderManager::getShader(const std::string& name) const {
    auto it = shaders.find(name);
    if (it != shaders.end())
        return it->second;
    return nullptr;
}