#pragma once

#include "maths_funcs.h"
#include <vector>
#include <string>
#include <GL/glew.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

using namespace std;

// 定义模型数据结构
typedef struct {
    size_t mPointCount = 0;
    vector<vec3> mVertices;
    vector<vec3> mNormals;
    vector<vec2> mTextureCoords;
    vec3 diffuseColor = vec3(1.0f, 1.0f, 1.0f); // 默认颜色（白色）
    bool hasColor = false; // 指示是否定义了颜色
} ModelData;

// 定义模型结构
struct Model {
    string name;
    ModelData data;
    vec3 position;
    float rotationY;
    GLuint vao; // 该模型的 VAO
    GLuint textureID; //纹理
    bool hasTexture; // 指示是否有纹理
};

// 定义模型部件结构
struct ModelPart {
    ModelData data;
    GLuint vao;
};

// 定义鱼模型结构
struct FishModel {
    ModelPart body;
    ModelPart fin;
    vec3 position;
    float rotationY;
    vec3 direction; // 游泳方向
    float finAngle;
    bool hasTexture;
    GLuint textureID;
    vec3 color; // 颜色属性
};

class Mesh {
public:
    static ModelData load_obj_mesh(const char* file_name);
    static ModelData load_mesh(const char* file_name);
    static Model load_model(const char* file_name, vec3 position, float rotationY, const char* textureFile, int scale);

private:
    static GLuint loadTexture(const char* textureFile); // 加载纹理的函数声明
};
