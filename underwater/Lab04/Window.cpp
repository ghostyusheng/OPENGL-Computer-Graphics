
#include "Window.h"
#include <stdio.h>
#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Render.h"
#include "Keyboard.h"


// 构造函数实现
//Window::Window(const std::string& name, int value)
//    : name(name), value(value) {}

void displayShow() {
    Render::render();
    glutSwapBuffers();
}

void sceneShow() {
    glutPostRedisplay(); // Request a redraw to update the display
}

void Window::init(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Be humble, be brave, be kind and be faithful ");
    //glutFullScreen();

    glutDisplayFunc(displayShow);
    //glutIdleFunc(sceneShow);
    glutKeyboardFunc(Keyboard::keypress);
    glutMouseFunc(Keyboard::mouseButton);
    glutMotionFunc(Keyboard::mouseMotion);

    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return;
    }

    Render r;
    r.initResource();

    glutMainLoop();
}

