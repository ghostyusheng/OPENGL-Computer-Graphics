#include "Render.h"
#include "Shader.h"
#include "Mesh.h"
#include "Shader.h"
#include "Window.h"
#include "global.h"
#include "Log.h"

void Render::renderModel(Model& model) {
    // �л���ɫ��������ģ�����ͣ�
    string shaderName = model.name;
    //Log::log(Log::Level::INFO, Str("-> {}", shaderName));
    GLuint shaderProgram = Shader::shaders["model"]; // ����ÿ��ģ�Ͷ���һ��shaderName
    glUseProgram(shaderProgram);

    // ��VAO
    glBindVertexArray(model.vao);

    // ����ģ�;���
    mat4 modelMatrix = identity_mat4();
    modelMatrix = translate(modelMatrix, model.position);
    modelMatrix = rotate_y_deg(modelMatrix, model.rotationY);
    int matrix_location = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelMatrix.m);

    // ����ɫ
    int color_location = glGetUniformLocation(shaderProgram, "diffuseColor");
    Render::bindColor(color_location, model.data.diffuseColor, model.data.hasColor);

    if (model.hasTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model.textureID);
        glUniform1i(glGetUniformLocation(Shader::shaders["model"], "objectTexture"), 0);
    }

    // ����useTexture��uniform����
    glUniform1i(glGetUniformLocation(Shader::shaders["model"], "useTexture"), model.hasTexture);


    // ����������У�
    if (model.textureID != -1) {
        Render::bindTexture(model.textureID);
    }

    // ����ģ��
    if ("terrain1.obj" == model.name || "assets/qst.obj" == model.name) {
        glDrawArrays(GL_QUADS, 0, model.data.mPointCount);
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, model.data.mPointCount);
    }
}


void Render::render() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ����ͶӰ����
    glUseProgram(Shader::shaders["model"]); // ʹ��Ĭ����ɫ��
    int proj_mat_location = glGetUniformLocation(Shader::shaders["model"], "proj");
    mat4 persp_proj = perspective(45.0f, (float)Window::width / (float)Window::height, 0.1f, 1000.0f);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);

    // ������ͼ����
    int view_mat_location = glGetUniformLocation(Shader::shaders["model"], "view");
    mat4 view = identity_mat4();
    view = translate(view, vec3(0.0f, 0.0f, -mycameraDistance));
    view = translate(view, -mycameraPosition);
    view = rotate_x_deg(view, mycameraRotationX);
    view = rotate_y_deg(view, mycameraRotationY);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);

    // ��Ⱦÿ��ģ��
    for (auto& model : objs) {
        renderModel(model);
    }
}


void Render::initResource()
{
    Shader::CompileShaders("model", "simpleVertexShader.txt", "simpleFragmentShader.txt");
    //Shader::CompileShaders("simple", "1.glsl", "2.glsl");

    initModel();
}

void Render::initModel() {
    objs.push_back(
        Mesh::load_model(
            "red_cube.dae",
            vec3(0.0f, -3.0f, 0.0f),
            0.0f,
            nullptr, 1)
    );

    //objs.push_back(
    //    Mesh::load_model(
    //        "terrain1.obj",
    //        vec3(0.0f, -12.0f, -10.0f),
    //        30.0f,
    //        "assets/stone2.jpg", 8)
    //);

    objs.push_back(
        Mesh::load_model(
            "assets/new/dizuo.dae",
            vec3(0.0f, -12.0f, -10.0f),
            215.0f,
            nullptr, 1)
    );
}

void Render::bindColor(int colorLocation, const vec3& color, bool hasColor) {
    if (colorLocation != -1) {
        if (hasColor) {
            glUniform3fv(colorLocation, 1, &color.v[0]);
        }
        else {
            vec3 defaultColor(1.0f, 1.0f, 1.0f); // ��ɫ
            glUniform3fv(colorLocation, 1, &defaultColor.v[0]);
        }
    }
    else {
        std::cerr << "Warning: diffuseColor uniform not found!" << std::endl;
    }
}

void Render::bindTexture(int textureID) {
    if (textureID != -1) {
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
}
