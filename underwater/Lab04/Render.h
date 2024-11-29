
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

class Render {
public:
    static void render();
    void initResource();
};


#endif
