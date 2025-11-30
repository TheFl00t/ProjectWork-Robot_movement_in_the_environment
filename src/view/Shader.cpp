#include "Shader.h"

Shader::Shader(std::string vertex_shader_fileName, std::string fragment_shader_fileName) {
    // Шукаємо файли шейдерів і завантажуємо GLSL код із них
    ID = make_shader(
        getFilePath(vertex_shader_fileName),
        getFilePath(fragment_shader_fileName)    
    );
}

Shader::~Shader() {
    glDeleteProgram(ID);
}

// Пошук файлу: шукає біля .exe, і в папці src проекту
std::string Shader::getFilePath(const std::string& fileName) {
    std::filesystem::path currentPath = std::filesystem::current_path();
    
    // Варіант 1: Запуск із build/Debug/
    std::filesystem::path pathFromExe = currentPath / "../../src/shaders" / fileName;
    if (std::filesystem::exists(pathFromExe)) {
        std::cout << "[Shader] Found file: " << fileName << " at: " << pathFromExe.string() << std::endl;
        return pathFromExe.string();
    }
    
    // Варіант 2: Запуск із кореня проекту (VSCode)
    std::filesystem::path pathFromRoot = currentPath / "src/shaders" / fileName;
    if (std::filesystem::exists(pathFromRoot)) {
        std::cout << "[Shader] Found file: " << fileName << " at: " << pathFromRoot.string() << std::endl;
        return pathFromRoot.string();
    }

    std::cerr << "[Shader] CRITICAL: File not found! Checked:\n"
              << "\t1. " << pathFromRoot << "\n"
              << "\t2. " << pathFromExe << std::endl;

    return ""; // Обробимо у make_module
}

GLuint Shader::make_module(const std::string& filepath, GLuint module_type) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[Shader] Failed to open file: " << filepath << std::endl;
        return 0;
    }

    std::stringstream bufferedLines;
    std::string line;
    while (std::getline(file, line)) {
        bufferedLines << line << "\n";
    }
    file.close();

    std::string shaderSource = bufferedLines.str();
    const char* shaderSrc = shaderSource.c_str(); 

    GLuint shaderModule = glCreateShader(module_type);
    glShaderSource(shaderModule, 1, &shaderSrc, NULL);
    glCompileShader(shaderModule);
    
    // Перевірка помилок компіляції
    GLint success;
    glGetShaderiv(shaderModule, GL_COMPILE_STATUS, &success);
    if (!success) {
        char errorLog[1024];
        glGetShaderInfoLog(shaderModule, 1024, NULL, errorLog);
        std::cout << "[Shader] Compile error in " << filepath << ":\n" << errorLog << std::endl;
    }
    
    return shaderModule;
}

GLuint Shader::make_shader(const std::string& vertex_filepath, const std::string& fragment_filepath) {
    std::vector<GLuint> modules;
    modules.push_back(make_module(vertex_filepath, GL_VERTEX_SHADER));
    modules.push_back(make_module(fragment_filepath, GL_FRAGMENT_SHADER));

    GLuint shader = glCreateProgram();
    for (GLuint shaderModule : modules) {
        glAttachShader(shader, shaderModule);
    }
    glLinkProgram(shader);

    // Перевірка помилок лінковки
    GLint success;
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
        char errorLog[1024];
        glGetProgramInfoLog(shader, 1024, NULL, errorLog);
        std::cout << "[Shader] Link error:\n" << errorLog << std::endl;
    }

    for (GLuint shaderModule : modules) {
        glDeleteShader(shaderModule);
    }

    return shader;
}

void Shader::use() const {
    glUseProgram(ID);
}

// --- Uniform Setters ---
void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}