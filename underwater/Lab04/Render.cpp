#include "Render.h"
#include "Shader.h"
#include "Mesh.h"
#include "Shader.h"
#include "Window.h"
#include "global.h"


void Render::render()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // Set background color to a deep blue
    glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(Shader::shaders["model"]);

    int view_mat_location = glGetUniformLocation(Shader::shaders["model"], "view");
    int proj_mat_location = glGetUniformLocation(Shader::shaders["model"], "proj");

    mat4 persp_proj = perspective(45.0f, (float)Window::width / (float)Window::height, 0.1f, 1000.0f);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);

    mat4 view = identity_mat4();
    view = translate(view, vec3(0.0f, 0.0f, -mycameraDistance)); // Apply zoom (camera distance)
    view = translate(view, -mycameraPosition);                   // Apply camera position for WASD
    view = rotate_x_deg(view, mycameraRotationX);                // Vertical rotation
    view = rotate_y_deg(view, mycameraRotationY);                // Horizontal rotation

    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);

    for (const auto& model : objs) {
        glBindVertexArray(model.vao);
        mat4 modelMatrix = identity_mat4();
        modelMatrix = translate(modelMatrix, model.position);
        int matrix_location = glGetUniformLocation(Shader::shaders["model"], "model");
        glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelMatrix.m);
        //printf("points: %d \n", model.data.mPointCount);
        glDrawArrays(GL_TRIANGLES, 0, model.data.mPointCount);

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
}
