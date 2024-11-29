#ifndef SHADER_H
#define SHADER_H

#include <map>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

using namespace std;

class Shader {
public:
    static map<string, unsigned int> shaders;

    static char* readShaderSource(const char* shaderFile);
    static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
    static void CompileShaders(string name, string vertex_file, string fragment_file);
};

#endif // SHADER_H
