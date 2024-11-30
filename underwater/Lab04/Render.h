
#ifndef RENDER_H
#define RENDER_H

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

#include <string>
#include <random>
#include <iostream>
#include "Mesh.h"

class Render {
public:
    static void renderModel(Model& model);
    static void render();
    static void initModel();
    static void bindColor(int colorLocation, const vec3& color, bool hasColor);
    static void bindTexture(int textureID);
    void initResource();
};


#endif
