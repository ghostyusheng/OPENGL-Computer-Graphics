// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>


// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

#include <random>

// Project includes
#include "maths_funcs.h"
#include "corecrt_math_defines.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <map>

vec3 cameraPosition(0.0f, 0.0f, 10.0f);
float cameraRotationY = 0.0f; // For rotation around the Y-axis
float cameraRotationX = 0.0f; // New for vertical rotation
float cameraDistance = 10.0f; // Camera distance for zoom control
bool leftMousePressed = false;
int lastMouseX, lastMouseY;

vec3 fishPosition(0.0f, 0.0f, 0.0f);
float traceRadius = 15.0f;  // Radius of the swimming circle
float traceSpeed = 0.5f;   // Speed of the fish movement
float angle = 0.0f;        // Angle along the path
static float finAngle = 0.0f; // Declare finAngle as static/global

float squidSpeed = 0.5f; // 鱿鱼移动速度
float sharkSpeed = 1.8f; // 鲨鱼移动速度
float seahorseDirection = 1.0f; // 鱿鱼方向（1:向上，-1:向下）
float sharkDirectionX = 1.0f; // 鲨鱼水平移动方向
float sharkRotationY = 0.0f;
float squidDirectionX = 1.1f; // 鲨鱼水平移动方向

float aincradRotationX = 0.0f; // 用于存储aincrad.dae的旋转角度




static std::default_random_engine e;

float randomFloat(float min, float max) {
    // 确保 min 小于 max
    if (min > max) {
        std::swap(min, max);
    }
    // 生成随机浮点数
    float scale = static_cast<float>(rand()) / static_cast<float>(RAND_MAX); // [0, 1]
    return min + scale * (max - min); // [min, max]
}

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
#define MESH_NAME "monkey.dae"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/


#pragma region SimpleTypes
typedef struct {
    size_t mPointCount = 0;
    std::vector<vec3> mVertices;
    std::vector<vec3> mNormals;
    std::vector<vec2> mTextureCoords;
    vec3 diffuseColor = vec3(1.0f, 1.0f, 1.0f); // Default color (white)
    bool hasColor = false; // Indicates if a color is defined
} ModelData;

struct Model {
    std::string name;
    ModelData data;
    vec3 position;
    float rotationY;
    GLuint vao; // VAO for this specific model
    GLuint textureID;
    bool hasTexture; // 新增的布尔值，用于指示是否有纹理
};

struct ModelPart {
    ModelData data;
    GLuint vao;
};

struct FishModel {
    ModelPart body;
    ModelPart fin;
    vec3 position;
    float rotationY;
    vec3 direction; // New attribute for swimming direction
    float finAngle;
    bool hasTexture;
    GLuint textureID;
    vec3 color; // Add color attribute
};

std::vector<Model> models; // Vector to hold multiple models
std::vector<FishModel> fishModels; // Vector to hold multiple models
#pragma endregion SimpleTypes

using namespace std;

std::map<std::string, unsigned int> shaders;

int width = 800;
int height = 600;

GLuint loc1, loc2;

GLuint textureID;
Model terrain;

GLuint loadTexture(const char* filePath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 设置纹理环绕方式和过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 加载纹理数据
    int width, height, channels;
    unsigned char* data = stbi_load(filePath, &width, &height, &channels, 0);
    if (data) {
        GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;

        // 将纹理数据上传到 OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        std::cout << "Texture loaded: " << filePath << " (" << width << "x" << height << ")" << std::endl;
        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}

int  channels;


#pragma region MESH LOADING
ModelData load_obj_mesh(const char* file_name) {
    ModelData modelData;

    const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate | aiProcess_PreTransformVertices);
    if (!scene) {
        fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
        return modelData;
    }

    for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
        const aiMesh* mesh = scene->mMeshes[m_i];
        modelData.mPointCount += mesh->mNumVertices;
        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions()) {
                const aiVector3D* vp = &(mesh->mVertices[v_i]);
                modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
            }
            if (mesh->HasNormals()) {
                const aiVector3D* vn = &(mesh->mNormals[v_i]);
                modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
            }
            if (mesh->HasTextureCoords(0)) {
                const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
                modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
            }


        }
    }

    aiReleaseImport(scene);
    return modelData;
}


ModelData load_mesh(const char* file_name) {
    ModelData modelData;

    const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate | aiProcess_PreTransformVertices);
    if (!scene) {
        fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
        return modelData;
    }
    else {
        printf("=> loaded: %s \n ", file_name);
    }

    for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
        const aiMesh* mesh = scene->mMeshes[m_i];
        modelData.mPointCount += mesh->mNumVertices;
        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions()) {
                const aiVector3D* vp = &(mesh->mVertices[v_i]);
                modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
            }
            if (mesh->HasNormals()) {
                const aiVector3D* vn = &(mesh->mNormals[v_i]);
                modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
            }
            if (mesh->HasTextureCoords(0)) {
                const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
                modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
            }

            // Load the diffuse color if no texture coordinates are available
            if (scene->mMaterials[mesh->mMaterialIndex]) {
                //std::cout << "material color: " << std::endl;
                aiColor4D diffuse;
                if (AI_SUCCESS == aiGetMaterialColor(scene->mMaterials[mesh->mMaterialIndex], AI_MATKEY_COLOR_DIFFUSE, &diffuse)) {
                    modelData.diffuseColor = vec3(diffuse.r, diffuse.g, diffuse.b);
                    //print(vec3(diffuse.r, diffuse.g, diffuse.b));
                    modelData.hasColor = true; // Set flag to indicate the presence of color
                }
            }
        }
    }

    printf("=> count : %d \n", modelData.mPointCount);

    aiReleaseImport(scene);
    return modelData;
}

Model load_model(const char* file_name, vec3 position, float rotationY, const char* textureFile, int scale) {
    Model model;
    std::string str;
    str = file_name;
    model.name = str;

    // Check file extension
    std::string fileStr(file_name);
    if (fileStr.substr(fileStr.find_last_of(".") + 1) == "obj") {
        model.data = load_obj_mesh(file_name);
    }
    else {
        model.data = load_mesh(file_name); // 现有的 DAE 加载
    }

    model.position = position;
    model.rotationY = rotationY;
    model.hasTexture = false;

    if (textureFile != nullptr && strlen(textureFile) > 0) {
        model.textureID = loadTexture(textureFile);
        model.hasTexture = true;
    }

    // 生成和绑定 VAO
    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);

    // 生成 VBOs
    GLuint vp_vbo, vn_vbo;
    glGenBuffers(1, &vp_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glBufferData(GL_ARRAY_BUFFER, model.data.mPointCount * sizeof(vec3), &model.data.mVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &vn_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glBufferData(GL_ARRAY_BUFFER, model.data.mPointCount * sizeof(vec3), &model.data.mNormals[0], GL_STATIC_DRAW);

    // 链接顶点位置
    glEnableVertexAttribArray(loc1);
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // 链接顶点法线
    glEnableVertexAttribArray(loc2);
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    if (!model.data.mTextureCoords.empty()) {
        // 缩放纹理坐标以实现纹理重复
        for (auto& texCoord : model.data.mTextureCoords) {
            texCoord.v[0] *= scale;  // 将此值设为你想要的重复倍数
            texCoord.v[1] *= scale;
        }

        GLuint vt_vbo;
        glGenBuffers(1, &vt_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
        glBufferData(GL_ARRAY_BUFFER, model.data.mPointCount * sizeof(vec2), &model.data.mTextureCoords[0], GL_STATIC_DRAW);
        GLint loc3 = glGetAttribLocation(shaders["model"], "vertex_texcoord");
        glEnableVertexAttribArray(loc3);
        glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    glBindVertexArray(0); // 完成设置后解绑 VAO

    return model;
}


FishModel load_fish_model(const char* file_name, vec3 position, float rotationY, const char* textureFile) {
    FishModel fishModel;
    fishModel.position = position;
    fishModel.rotationY = rotationY;

    // Generate a random color
    fishModel.color = vec3(randomFloat(0, 255) / 255.0f, randomFloat(0, 255) / 255.0f, randomFloat(0, 255) / 255.0f);
    // Debug output to check color
    std::cout << "Fish Color: (" << fishModel.color.v[0] << ", " << fishModel.color.v[1] << ", " << fishModel.color.v[2] << ")\n";

    fishModel.hasTexture = false;

    if (textureFile != nullptr && strlen(textureFile) > 0) {
        fishModel.textureID = loadTexture(textureFile);
        fishModel.hasTexture = true;
    }

    const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate | aiProcess_PreTransformVertices);
    if (!scene) {
        fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
        return fishModel;
    }
    else {
        fprintf(stderr, "Success: reading mesh %s\n", file_name);
    }

    std::cout << "Mesh count: " << scene->mNumMeshes << std::endl;
    printf("-> loaded fish texture? %d \n", fishModel.hasTexture);

    // Iterate through each mesh in the scene
    for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
        const aiMesh* mesh = scene->mMeshes[m_i];

        ModelData modelData;
        modelData.mPointCount = mesh->mNumVertices;

        // Populate vertex and normal data
        for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
            if (mesh->HasPositions()) {
                const aiVector3D* vp = &(mesh->mVertices[v_i]);
                modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
            }
            if (mesh->HasNormals()) {
                const aiVector3D* vn = &(mesh->mNormals[v_i]);
                modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
            }
        }

        // Debug output for mesh information
        std::cout << "Mesh Index: " << m_i << ", Vertex Count: " << mesh->mNumVertices << std::endl;

        ModelPart* part = nullptr;

        // Assign part based on mesh index
        if (m_i == 0) { // Assuming the first mesh is the body
            part = &fishModel.body;
        }
        else if (m_i == 1) { // Assuming the second mesh is the fin
            printf("---> Fin part found! \n");
            part = &fishModel.fin;
        }

        // Debug output for part assignment
        std::cout << "Part Pointer: " << part << std::endl;

        if (part) {
            part->data = modelData;

            // Create VAO and VBOs for the mesh part
            glGenVertexArrays(1, &part->vao);
            glBindVertexArray(part->vao);

            GLuint vp_vbo, vn_vbo, vt_vbo;
            glGenBuffers(1, &vp_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
            glBufferData(GL_ARRAY_BUFFER, part->data.mPointCount * sizeof(vec3), &part->data.mVertices[0], GL_STATIC_DRAW);

            glGenBuffers(1, &vn_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
            glBufferData(GL_ARRAY_BUFFER, part->data.mPointCount * sizeof(vec3), &part->data.mNormals[0], GL_STATIC_DRAW);

            // Link vertex positions
            glEnableVertexAttribArray(loc1);
            glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
            glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

            // Link vertex normals
            glEnableVertexAttribArray(loc2);
            glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
            glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

            glBindVertexArray(0); // Unbind VAO when done
        }
    }

    aiReleaseImport(scene);
    return fishModel;
}


void render_fish(const FishModel& fishModel) {
    /*std::cout << "fishModel: " << std::endl;
    print(fishPosition);
    std::cout << "body vao: " << std::endl;*/

    // Set color uniform
    glUniform3fv(glGetUniformLocation(shaders["model"], "fishColor"), 1, &fishModel.color.v[0]);

    // Set up body transformation
    mat4 bodyModel = identity_mat4();
    bodyModel = translate(bodyModel, fishModel.position);
    bodyModel = rotate_y_deg(bodyModel, fishModel.rotationY);

    // Check if model has a color and no texture
    int color_location = glGetUniformLocation(shaders["model"], "diffuseColor");
    glUniform1i(glGetUniformLocation(shaders["model"], "useTexture"), 0);
    //printf("has color : %d \n", model.data.hasColor);
    //print(model.data.diffuseColor);

    // 检查 color_location 是否有效
    if (color_location != -1) {
        //print(fishModel.color);
        glUniform3fv(color_location, 1, &fishModel.color.v[0]);
    }
    else {
        std::cerr << "Warning: diffuseColor uniform not found!" << std::endl;
    }

    int model_location = glGetUniformLocation(shaders["model"], "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, bodyModel.m);

    glBindVertexArray(fishModel.body.vao);
    glDrawArrays(GL_TRIANGLES, 0, fishModel.body.data.mPointCount);

    // Set up fin transformation (hierarchical: start with body’s transform)
    //printf("render_fish fin angle: %f \n", fishModel.finAngle);

    mat4 finModel = bodyModel;
    finModel = rotate_z_deg(finModel, fishModel.finAngle);  // Apply oscillation to fin
    glUniformMatrix4fv(model_location, 1, GL_FALSE, finModel.m);

    glBindVertexArray(fishModel.fin.vao);



    glDrawArrays(GL_TRIANGLES, 0, fishModel.fin.data.mPointCount);

    //printf("fish has texture: %d", fishModel.hasTexture);

   /* if (fishModel.hasTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fishModel.textureID);
        glUniform1i(glGetUniformLocation(shaderProgramID, "objectTexture"), 0);
    }*/

    // 设置useTexture的uniform变量
    //glUniform1i(glGetUniformLocation(shaderProgramID, "useTexture"), fishModel.hasTexture);
}

#pragma endregion MESH LOADING

#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
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

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) {
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

void CompileShaders(string name, string vertex_file, string fragment_file) {
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

    shaders[name] = shaderProgramID;
    return;
}
#pragma endregion SHADER_FUNCTIONS



void display() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // Set background color to a deep blue
    glClearColor(0.0f, 0.0f, 0.3f, 1.0f); // Adjust values to get the desired color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable fog
    glEnable(GL_FOG);
    glFogf(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 5.0f);    // Start of fog effect (closer for dense water effect)
    glFogf(GL_FOG_END, 50.0f);     // End of fog effect
    float fogColor[4] = { 0.0f, 0.2f, 0.3f, 1.0f }; // Blue-greenish color for fog
    glFogfv(GL_FOG_COLOR, fogColor);

    glUseProgram(shaders["model"]);

    int view_mat_location = glGetUniformLocation(shaders["model"], "view");
    int proj_mat_location = glGetUniformLocation(shaders["model"], "proj");

    mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);

    mat4 view = identity_mat4();
    view = translate(view, vec3(0.0f, 0.0f, -cameraDistance)); // Apply zoom (camera distance)
    view = translate(view, -cameraPosition);                   // Apply camera position for WASD
    view = rotate_x_deg(view, cameraRotationX);                // Vertical rotation
    view = rotate_y_deg(view, cameraRotationY);                // Horizontal rotation

    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);

    for (const auto& model : models) {
        glBindVertexArray(model.vao);

        if (model.hasTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, model.textureID);
            glUniform1i(glGetUniformLocation(shaders["model"], "objectTexture"), 0);
        }

        // 设置useTexture的uniform变量
        glUniform1i(glGetUniformLocation(shaders["model"], "useTexture"), model.hasTexture);

        mat4 modelMatrix = identity_mat4();
        if ("assets/shark3.dae" == model.name) {
            // 创建鲨鱼的变换矩阵
            modelMatrix = rotate_y_deg(modelMatrix, model.rotationY + sharkRotationY);
        }
        else if (model.name == "assets/aincrad.dae") {
            modelMatrix = rotate_y_deg(modelMatrix, aincradRotationX); // 应用Y轴旋转
        } else {
            modelMatrix = rotate_y_deg(modelMatrix, model.rotationY);
        }
        
        modelMatrix = translate(modelMatrix, model.position);

        int matrix_location = glGetUniformLocation(shaders["model"], "model");

  

        glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelMatrix.m);


        // Check if model has a color and no texture
        int color_location = glGetUniformLocation(shaders["model"], "diffuseColor");
        //printf("has color : %d \n", model.data.hasColor);
        //print(model.data.diffuseColor);

        // 检查 color_location 是否有效
        if (color_location != -1) {
            // 根据 hasColor 的值选择颜色
            if (model.data.hasColor) {
                glUniform3fv(color_location, 1, &model.data.diffuseColor.v[0]);
            }
            else {
                // 传递一个默认颜色，例如白色
                vec3 defaultColor(1.0f, 1.0f, 1.0f); // 白色
                glUniform3fv(color_location, 1, &defaultColor.v[0]);
            }
        }
        else {
            std::cerr << "Warning: diffuseColor uniform not found!" << std::endl;
        }


        //std::cout << "name: " + model.name << std::endl;
        if ("terrain1.obj" == model.name || "assets/qst.obj" == model.name) {
            glDrawArrays(GL_QUADS, 0, model.data.mPointCount);
        }
        else {
            glDrawArrays(GL_TRIANGLES, 0, model.data.mPointCount);
        }
    }

    //glDrawArrays(GL_QUADS, 0, terrain.data.mPointCount);

    mat4 bodyModel = identity_mat4();
    bodyModel = translate(bodyModel, fishModels[0].position);
    bodyModel = rotate_y_deg(bodyModel, fishModels[0].rotationY);

    int model_location = glGetUniformLocation(shaders["model"], "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, bodyModel.m);

    for (const auto& model : fishModels) {
        render_fish(model);
    }



    glUseProgram(shaders["simple"]);

    mat4 persp_proj2 = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaders["simple"], "proj"), 1, GL_FALSE, persp_proj2.m);

    mat4 view2 = identity_mat4();
    view = translate(view, vec3(0.0f, 0.0f, -5.0f)); // 适当设置相机位置
    glUniformMatrix4fv(glGetUniformLocation(shaders["simple"], "view"), 1, GL_FALSE, view2.m);

    glutSwapBuffers();
}

GLfloat rotate_y = 0.0f;


void updateScene() {
    static DWORD last_time = 0;
    static float maxFinAngle = 1;
    static float finOscillationSpeed = 0.15f;
    DWORD curr_time = timeGetTime();
    if (last_time == 0)
        last_time = curr_time;
    float delta = (curr_time - last_time) * 0.001f;
    last_time = curr_time;

    // 更新aincrad的旋转角度
    aincradRotationX += 10.0f * delta; // 每秒旋转10度（可以根据需要调整速度）


    // Update each fish model
    for (auto& fish : fishModels) {
        // Update fish position based on its direction (horizontal movement only)
        fish.position.v[0] += fish.direction.v[0] * traceSpeed * delta; // x
        fish.position.v[1] = fish.direction.v[1]; // Keep y position constant for horizontal movement
        fish.position.v[2] += fish.direction.v[2] * traceSpeed * delta; // z

        // Check if the fish has reached a certain distance to reverse direction
        if (fish.position.v[0] >= traceRadius || fish.position.v[0] <= -traceRadius) {
            fish.direction.v[0] = -fish.direction.v[0]; // Reverse direction
        }
        if (fish.position.v[2] >= traceRadius || fish.position.v[2] <= -traceRadius) {
            fish.direction.v[2] = -fish.direction.v[2]; // Reverse direction
        }

        // Update fin angle for animation
        fish.finAngle = maxFinAngle * sinf(curr_time * finOscillationSpeed);
    }

    // 更新鱿鱼的Y坐标
    for (auto& fish : models) {
        if (fish.name == "assets/seahorse.dae") { // 假设鱿鱼模型名称为" squid"
            //fish.position.v[1] += squidSpeed * seahorseDirection * delta; // 上下移动
            if (fish.position.v[1] >= -3.0f || fish.position.v[1] <= 3.0f) { // 设定上下边界
                fish.position.v[1] = 1.0f;
                seahorseDirection *= -1; // 反转方向
            }
            // 添加抖动效果
            fish.position.v[1] += (rand() % 10 - 5) * 0.03f * seahorseDirection; // 小范围抖动
        }

        // 更新鲨鱼的X坐标
        if (fish.name == "assets/shark3.dae") { // 假设鲨鱼模型名称为"shark"
            fish.position.v[0] += sharkSpeed * sharkDirectionX * delta; // 水平移动
            if (fish.position.v[0] >= 10.0f || fish.position.v[0] <= -1.0f) { // 设定水平边界
                sharkDirectionX *= -1; // 反转方向
                sharkRotationY += 180.0f; // 改变旋转角度
                // 更新鲨鱼的旋转

            }
        }
        if (fish.name == "assets/squid.dae") { // 假设鲨鱼模型名称为"shark"
            fish.position.v[0] += squidSpeed * squidDirectionX * delta; // 水平移动
            if (fish.position.v[0] >= 15.0f || fish.position.v[0] <= -25.0f) { // 设定水平边界
                squidDirectionX *= -1; // 反转方向
            }
        }
    }


    glutPostRedisplay(); // Request a redraw to update the display
}

void init() {
    CompileShaders("model", "simpleVertexShader.txt", "simpleFragmentShader.txt");
    CompileShaders("simple", "1.glsl", "2.glsl");

    loc1 = glGetAttribLocation(shaders["model"], "vertex_position");
    loc2 = glGetAttribLocation(shaders["model"], "vertex_normal");

    // 加载高度图模型
    //terrain = load_heightmap_model("heightmap.png", vec3(0.0f, -2.0f, -10.0f), 0.0f, 1.0f);

    models.push_back(
        load_model(
        "terrain1.obj",
        vec3(0.0f, -12.0f, -10.0f),
        30.0f,
        "assets/stone2.jpg", 8)
    );
    models.push_back(
        load_model(
            "assets/aincrad.dae",
            vec3(10.0f, 30.0f, -70.0f),
            0.0f,
            nullptr, 1)
    );

    models.push_back(
        load_model(
            "assets/tkr.dae",
            vec3(-8.0f, -10, -9.0f),
            275.0f,
            "assets/metal1.jpg", 1)
    );

    for (int i = 0;i < 5;i++) {
        models.push_back(
            load_model(
                "assets/white_coral.dae",
                vec3(i+10, -10.0f, -(10+i)),
                30+i,
                nullptr, 1)
        );
    }
   
    for (int i = 0;i < 3;i++) {
        models.push_back(
            load_model(
                "assets/red_coral.dae",
                vec3(i + 8, -10.0f, -(10 + i + 5)),
                30 + i,
                nullptr, 1)
        );
    }
    models.push_back(
        load_model(
            "assets/qst.obj",
            vec3(10.0f, -24.0f, 18.0f),
            45.0f,
            "assets/qst.png", 1)
    );

      models.push_back(
            load_model(
                "assets/weed.dae",
                vec3(-10.0f, -12.0f, -30.0f),
                45.0f,
                nullptr, 1)
        );
      models.push_back(
          load_model(
              "assets/weed.dae",
              vec3(5.0f, -12.0f, -30.0f),
              15.0f,
              nullptr, 1)
      );
   

    models.push_back(
        load_model(
            "assets/shark3.dae",
            vec3(0.0f, 0.0f, -3.0f),
            45.0f,
            nullptr, 1)
    );


    models.push_back(
        load_model(
            "assets/seahorse.dae",
            vec3(30.0f, 20.0f, -40.0f),
            15.0f,
            nullptr, 1)
    );

    models.push_back(
        load_model(
            "assets/squid.dae",
            vec3(0.0f, 10.0f, -10.0f),
            45.0f,
            nullptr, 1)
    );

    models.push_back(
        load_model(
            "assets/squid.dae",
            vec3(-3.0f, 14.0f, -12.0f),
            45.0f,
            nullptr, 1)
    );
    models.push_back(
        load_model(
            "assets/jiangyou.dae",
            vec3(12.0f, -12.0f, 3.0f),
            45.0f,
            nullptr, 1)
    );





    /*models.push_back(load_model("green_cube.dae", vec3(0.0f, 5.0f, -10.0f), -45.0f, nullptr));
    models.push_back(load_model("pic_cube.dae", vec3(0.0f, -4.0f, -10.0f), 30.0f, "diffuse.jpg"));*/
    //models.push_back(load_model("assets/fish2.dae", vec3(0.0f, -4.0f, -10.0f), 30.0f, "assets/fish.png"));

    // Initialize multiple fish models
    for (int i = 0; i < 100; ++i) {
        FishModel fish;
        fish = load_fish_model("assets/xxx.dae", vec3(randomFloat(-30, 15), randomFloat(-10,5), randomFloat(-10, -3)), rand() * 10 % 45, "assets/fish.png");
        fish.direction = vec3(randomFloat(1, 10), randomFloat(-4, 4), 0.0f); // Set initial swimming direction
        fishModels.push_back(fish);
    }

}


void keypress(unsigned char key, int x, int y) {
    float movementSpeed = 0.5f; // Speed of camera movement
    float rotationSpeed = 5.0f;  // Speed of camera rotation

    std::cout << "Key pressed: " << key << std::endl; // Debug output

    switch (key) {
    case 'w': // Move forward
        cameraPosition.v[2] -= movementSpeed;
        std::cout << "Moving Forward: " << cameraPosition.v[2] << std::endl;
        break;
    case 's': // Move backward
        cameraPosition.v[2] += movementSpeed;
        std::cout << "Moving Backward: " << cameraPosition.v[2] << std::endl;
        break;
    case 'a': // Move left
        cameraPosition.v[0] -= movementSpeed;
        std::cout << "Moving Left: " << cameraPosition.v[0] << std::endl;
        break;
    case 'd': // Move right
        cameraPosition.v[0] += movementSpeed;
        std::cout << "Moving Right: " << cameraPosition.v[0] << std::endl;
        break;
    case 'q': // Rotate left (yaw)
        cameraRotationY -= rotationSpeed;
        std::cout << "Rotating Left: " << cameraRotationY << std::endl;
        break;
    case 'e': // Rotate right (yaw)
        cameraRotationY += rotationSpeed;
        std::cout << "Rotating Right: " << cameraRotationY << std::endl;
        break;
    case 'r': // Move up
        cameraPosition.v[1] += movementSpeed;
        std::cout << "Moving Up: " << cameraPosition.v[1] << std::endl;
        break;
    case 'f': // Move down
        cameraPosition.v[1] -= movementSpeed;
        std::cout << "Moving Down: " << cameraPosition.v[1] << std::endl;
        break;
    }
    glutPostRedisplay(); // Request a redraw to update the display with changes
}


void mouseButton(int button, int state, int x, int y) {
    std::cout << button << std::endl;
    if (button == GLUT_LEFT_BUTTON) {
        leftMousePressed = (state == GLUT_DOWN);
        lastMouseX = x;
        lastMouseY = y;
    }
    else if (button == 3) { // Scroll up
        cameraDistance -= 1.0f;
        if (cameraDistance < 2.0f) cameraDistance = 2.0f; // Prevent too close zoom
    }
    else if (button == 4) { // Scroll down
        cameraDistance += 1.0f;
        if (cameraDistance > 50.0f) cameraDistance = 50.0f; // Limit zoom-out distance
    }
}

void mouseMotion(int x, int y) {
    if (leftMousePressed) {
        float dx = x - lastMouseX;
        float dy = y - lastMouseY;
        cameraRotationY += dx * 0.2f;
        cameraRotationX += dy * 0.2f;
        if (cameraRotationX > 89.0f) cameraRotationX = 89.0f; // Prevent flipping over top
        if (cameraRotationX < -89.0f) cameraRotationX = -89.0f; // Prevent flipping over bottom
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Hello Triangle");

    // 设置全屏
    //glutFullScreen();

    glutDisplayFunc(display);
    glutIdleFunc(updateScene);
    glutKeyboardFunc(keypress);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }
    init();

    // 注册显示和定时器函数
    glutDisplayFunc(display);

    glutMainLoop();
    return 0;
}
