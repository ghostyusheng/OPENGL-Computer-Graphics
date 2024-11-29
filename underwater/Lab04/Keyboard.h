#pragma once

#include <GL/glut.h>
#include <iostream>
#include "maths_funcs.h"

class Keyboard {
public:

    static void keypress(unsigned char key, int x, int y);
    static void mouseButton(int button, int state, int x, int y);
    static void mouseMotion(int x, int y);

    static void setCameraPosition(float x, float y, float z);
    static void setCameraRotation(float yaw, float pitch);
    static void setCameraDistance(float distance);

private:
    float movementSpeed;
    float rotationSpeed;
    float cameraDistance;
    vec3 cameraPosition; // Assuming vec3 is defined elsewhere
    float cameraRotationX;
    float cameraRotationY;
    bool leftMousePressed;
    int lastMouseX;
    int lastMouseY;
};
