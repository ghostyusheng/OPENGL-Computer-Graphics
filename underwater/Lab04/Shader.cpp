#include "Shader.h"

// 定义静态成员变量
map<string, unsigned int> Shader::shaders;

char* Shader::readShaderSource(const char* shaderFile) {
    FILE* fp;
    fopen_s(&fp, shaderFile, "rb");

    if (fp == NULL) { return NULL; }

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);
    char* buf = new char[size + 1];
    fread(buf, 1, size, fp);
    buf[size] = '\0';

    fclose(fp);

    return buf;
}

void Shader::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) {
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        std::cerr << "Error creating shader..." << std::endl;
        exit(1);
    }
    const char* pShaderSource = readShaderSource(pShaderText);

    glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
    glCompileShader(ShaderObj);
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024] = { '\0' };
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        std::cerr << "Error compiling shader: " << InfoLog << std::endl;
        exit(1);
    }
    glAttachShader(ShaderProgram, ShaderObj);
}

void Shader::CompileShaders(string name, string vertex_file, string fragment_file) {
    GLuint shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        std::cerr << "Error creating shader program..." << std::endl;
        exit(1);
    }

    AddShader(shaderProgramID, vertex_file.c_str(), GL_VERTEX_SHADER);
    AddShader(shaderProgramID, fragment_file.c_str(), GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { '\0' };
    glLinkProgram(shaderProgramID);
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
        exit(1);
    }

    glValidateProgram(shaderProgramID);
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
        exit(1);
    }
    glUseProgram(shaderProgramID);

    Shader::shaders[name] = shaderProgramID;
}
