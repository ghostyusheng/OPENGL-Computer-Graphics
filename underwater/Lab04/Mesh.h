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

// ����ģ�����ݽṹ
typedef struct {
    size_t mPointCount = 0;
    vector<vec3> mVertices;
    vector<vec3> mNormals;
    vector<vec2> mTextureCoords;
    vec3 diffuseColor = vec3(1.0f, 1.0f, 1.0f); // Ĭ����ɫ����ɫ��
    bool hasColor = false; // ָʾ�Ƿ�������ɫ
} ModelData;

// ����ģ�ͽṹ
struct Model {
    string name;
    ModelData data;
    vec3 position;
    float rotationY;
    GLuint vao; // ��ģ�͵� VAO
    GLuint textureID; //����
    bool hasTexture; // ָʾ�Ƿ�������
};

// ����ģ�Ͳ����ṹ
struct ModelPart {
    ModelData data;
    GLuint vao;
};

// ������ģ�ͽṹ
struct FishModel {
    ModelPart body;
    ModelPart fin;
    vec3 position;
    float rotationY;
    vec3 direction; // ��Ӿ����
    float finAngle;
    bool hasTexture;
    GLuint textureID;
    vec3 color; // ��ɫ����
};

class Mesh {
public:
    static ModelData load_obj_mesh(const char* file_name);
    static ModelData load_mesh(const char* file_name);
    static Model load_model(const char* file_name, vec3 position, float rotationY, const char* textureFile, int scale);

private:
    static GLuint loadTexture(const char* textureFile); // ��������ĺ�������
};
