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

// Project includes
#include "maths_funcs.h"

vec3 cameraPosition(0.0f, 0.0f, 10.0f);
float cameraRotationY = 0.0f; // For rotation around the Y-axis


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
} ModelData;

struct Model {
    ModelData data;
    vec3 position;
    float rotationY;
    GLuint vao; // VAO for this specific model
};

std::vector<Model> models; // Vector to hold multiple models
#pragma endregion SimpleTypes

using namespace std;
GLuint shaderProgramID;

int width = 800;
int height = 600;

GLuint loc1, loc2;

#pragma region MESH LOADING
ModelData load_mesh(const char* file_name) {
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
        }
    }

    aiReleaseImport(scene);
    return modelData;
}

Model load_model(const char* file_name, vec3 position, float rotationY) {
    Model model;
    model.data = load_mesh(file_name);
    model.position = position;
    model.rotationY = rotationY;

    // Generate and bind VAO for this model
    glGenVertexArrays(1, &model.vao);
    glBindVertexArray(model.vao);

    // Generate VBOs for vertex positions and normals
    GLuint vp_vbo, vn_vbo;
    glGenBuffers(1, &vp_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glBufferData(GL_ARRAY_BUFFER, model.data.mPointCount * sizeof(vec3), &model.data.mVertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &vn_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glBufferData(GL_ARRAY_BUFFER, model.data.mPointCount * sizeof(vec3), &model.data.mNormals[0], GL_STATIC_DRAW);

    // Link vertex positions
    glEnableVertexAttribArray(loc1);
    glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
    glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    // Link vertex normals
    glEnableVertexAttribArray(loc2);
    glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
    glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindVertexArray(0); // Unbind VAO when done setting it up

    return model;
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

GLuint CompileShaders() {
    shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        std::cerr << "Error creating shader program..." << std::endl;
        exit(1);
    }

    AddShader(shaderProgramID, "simpleVertexShader.txt", GL_VERTEX_SHADER);
    AddShader(shaderProgramID, "simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

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
    return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

void display() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shaderProgramID);

    int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
    int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");

    mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);

    mat4 view = identity_mat4();
    view = translate(view, -cameraPosition);
    view = rotate_y_deg(view, cameraRotationY);

    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);

    for (const auto& model : models) {
        glBindVertexArray(model.vao);


        mat4 modelMatrix = identity_mat4();
        modelMatrix = rotate_y_deg(modelMatrix, model.rotationY);
        modelMatrix = translate(modelMatrix, model.position);

        int matrix_location = glGetUniformLocation(shaderProgramID, "model");
        glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelMatrix.m);

        glDrawArrays(GL_TRIANGLES, 0, model.data.mPointCount);
    }

    glutSwapBuffers();
}

GLfloat rotate_y = 0.0f;


void updateScene() {
    static DWORD last_time = 0;
    DWORD curr_time = timeGetTime();
    if (last_time == 0)
        last_time = curr_time;
    float delta = (curr_time - last_time) * 0.001f;
    last_time = curr_time;

    rotate_y += 20.0f * delta;
    rotate_y = fmodf(rotate_y, 360.0f);

    glutPostRedisplay();
}

void init() {
    GLuint shaderProgramID = CompileShaders();

    loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
    loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");

    models.push_back(load_model("monkey.dae", vec3(0.0f, 0.0f, -10.0f), 45.0f));
    models.push_back(load_model("cube.dae", vec3(0.0f, 5.0f, -10.0f), -45.0f));
    models.push_back(load_model("cube_big.dae", vec3(0.0f, -5.0f, -10.0f), 45.0f));

}

void keypress(unsigned char key, int x, int y) {
    float movementSpeed = 0.5f;
    float rotationSpeed = 5.0f;

    switch (key) {
    case 'w': cameraPosition.v[2] -= movementSpeed; break;
    case 's': cameraPosition.v[2] += movementSpeed; break;
    case 'a': cameraPosition.v[0] -= movementSpeed; break;
    case 'd': cameraPosition.v[0] += movementSpeed; break;
    case 'q': cameraRotationY -= rotationSpeed; break;
    case 'e': cameraRotationY += rotationSpeed; break;
    case 'r': cameraPosition.v[1] += movementSpeed; break;
    case 'f': cameraPosition.v[1] -= movementSpeed; break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Hello Triangle");

    glutDisplayFunc(display);
    glutIdleFunc(updateScene);
    glutKeyboardFunc(keypress);

    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }
    init();
    glutMainLoop();
    return 0;
}
