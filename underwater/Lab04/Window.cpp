
#include "Window.h"
#include <stdio.h>
#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GL/freeglut.h>


// ���캯��ʵ��
//Window::Window(const std::string& name, int value)
//    : name(name), value(value) {}

void displayShow() {
    glutSwapBuffers();
}

void Window::init(int argc, char** argv)
{
    int width = 800;
    int height = 600;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Be humble, be brave, be kind and be faithful ");
    //glutFullScreen();

    glutDisplayFunc(displayShow);
    /*glutIdleFunc(updateScene);
    glutKeyboardFunc(keypress);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);*/

    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return;
    }
    //init();

    glutMainLoop();
}

