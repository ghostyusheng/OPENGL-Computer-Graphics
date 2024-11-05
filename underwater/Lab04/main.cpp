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
#include "corecrt_math_defines.h"

vec3 cameraPosition(0.0f, 0.0f, 10.0f);
float cameraRotationY = 0.0f; // For rotation around the Y-axis
float cameraRotationX = 0.0f; // New for vertical rotation
float cameraDistance = 10.0f; // Camera distance for zoom control
bool leftMousePressed = false;
int lastMouseX, lastMouseY;

vec3 fishPosition(0.0f, 0.0f, 0.0f);
float traceRadius = 5.0f;  // Radius of the swimming circle
float traceSpeed = 0.5f;   // Speed of the fish movement
float angle = 0.0f;        // Angle along the path
static float finAngle = 0.0f; // Declare finAngle as static/global


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

struct ModelPart {
    ModelData data;
    GLuint vao;
};

struct FishModel {
    ModelPart body;
    ModelPart fin;
    vec3 position;
    float rotationY;
};

std::vector<Model> models; // Vector to hold multiple models
std::vector<FishModel> fishModels; // Vector to hold multiple models
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

FishModel load_fish_model(const char* file_name, vec3 position, float rotationY) {
    FishModel fishModel;
    fishModel.position = position;
    fishModel.rotationY = rotationY;

    const aiScene* scene = aiImportFile(file_name, aiProcess_Triangulate | aiProcess_PreTransformVertices);
    if (!scene) {
        fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
        return fishModel;
    }

    std::cout << "Mesh count: " << scene->mNumMeshes << std::endl;

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
            part = &fishModel.fin;
        }

        // Debug output for part assignment
        std::cout << "Part Pointer: " << part << std::endl;

        if (part) {
            part->data = modelData;

            // Create VAO and VBOs for the mesh part
            glGenVertexArrays(1, &part->vao);
            glBindVertexArray(part->vao);

            GLuint vp_vbo, vn_vbo;
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


void render_fish(const FishModel& fishModel, float finAngle) {
    std::cout << "fishModel: " << std::endl;
    print(fishModel.position);
    std::cout << "body vao: " << std::endl;
    

    // Set up body transformation
    mat4 bodyModel = identity_mat4();
    bodyModel = translate(bodyModel, fishModel.position);
    bodyModel = rotate_y_deg(bodyModel, fishModel.rotationY);

    int model_location = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, bodyModel.m);

    glBindVertexArray(fishModel.body.vao);
    glDrawArrays(GL_TRIANGLES, 0, fishModel.body.data.mPointCount);

    // Set up fin transformation (hierarchical: start with body¡¯s transform)
    mat4 finModel = bodyModel;
    finModel = rotate_z_deg(finModel, finAngle);  // Apply oscillation to fin

    glUniformMatrix4fv(model_location, 1, GL_FALSE, finModel.m);

    glBindVertexArray(fishModel.fin.vao);
    glDrawArrays(GL_TRIANGLES, 0, fishModel.fin.data.mPointCount);
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
    view = translate(view, vec3(0.0f, 0.0f, -cameraDistance)); // Apply zoom (camera distance)
    view = translate(view, -cameraPosition);                   // Apply camera position for WASD
    view = rotate_x_deg(view, cameraRotationX);                // Vertical rotation
    view = rotate_y_deg(view, cameraRotationY);                // Horizontal rotation

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

    mat4 bodyModel = identity_mat4();
    bodyModel = translate(bodyModel, fishModels[0].position);
    bodyModel = rotate_y_deg(bodyModel, fishModels[0].rotationY);

    int model_location = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, bodyModel.m);

    //std::cout << fishModels[0].body.data.mPointCount << std::endl;

    /*glBindVertexArray(fishModels[0].body.vao);
    glDrawArrays(GL_TRIANGLES, 0, fishModels[0].body.data.mPointCount);*/

    for (const auto& model : fishModels) {
        render_fish(model, finAngle);
    }

    //// Fish model matrix
    //mat4 fishModel = identity_mat4();
    //fishModel = translate(fishModel, fishPosition); // Position on the path

    //// Make the fish face the movement direction by adjusting Y rotation
    //float facingAngle = angle * (180.0f / M_PI);  // Convert radians to degrees
    //fishModel = rotate_y_deg(fishModel, facingAngle);

    //int model_location = glGetUniformLocation(shaderProgramID, "model");
    //glUniformMatrix4fv(model_location, 1, GL_FALSE, fishModel.m);

    //// Draw fish model
    //glBindVertexArray(models[0].vao);
    //glDrawArrays(GL_TRIANGLES, 0, models[0].data.mPointCount);

    glutSwapBuffers();
}

GLfloat rotate_y = 0.0f;


void updateScene() {
    static DWORD last_time = 0;
    static float maxFinAngle = 10;
    static float finOscillationSpeed = 2.0f;
    DWORD curr_time = timeGetTime();
    if (last_time == 0)
        last_time = curr_time;
    float delta = (curr_time - last_time) * 0.001f;
    last_time = curr_time;

    angle += traceSpeed * delta; // Update angle based on speed
    if (angle >= 360.0f) angle -= 360.0f; // Keep angle in range

    // Calculate new position along circular path
    float x = traceRadius * cosf(angle);
    float z = traceRadius * sinf(angle);
    fishPosition = vec3(x, 0.0f, z); // Update fish position along trace

    finAngle = maxFinAngle * sinf(curr_time * finOscillationSpeed);


    glutPostRedisplay();
}


void init() {
    GLuint shaderProgramID = CompileShaders();

    loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
    loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");

    models.push_back(load_model("monkey.dae", vec3(0.0f, 0.0f, -10.0f), -45.0f));
    //models.push_back(load_model("cube.dae", vec3(0.0f, 5.0f, -10.0f), 0.0f));
    //models.push_back(load_model("simple_fish.dae", vec3(0.0f, -5.0f, -10.0f), 45.0f));
    fishModels.push_back(load_fish_model("simple_fish5.dae", vec3(0.0f, -5.0f, -10.0f), 45.0f));

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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Hello Triangle");

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
    glutMainLoop();
    return 0;
}
