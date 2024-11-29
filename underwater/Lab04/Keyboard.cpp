#include "global.h"
#include "Keyboard.h"
#include "maths_funcs.h"
#include "Render.h"



void Keyboard::keypress(unsigned char key, int x, int y) {
    std::cout << "Key pressed: " << key << std::endl; // Debug output

    switch (key) {
    case 'w': // Move forward
        mycameraPosition.v[2] -= mymovementSpeed;
        std::cout << "Moving Forward: " << mycameraPosition.v[2] << std::endl;
        break;
    case 's': // Move backward
        mycameraPosition.v[2] += mymovementSpeed;
        std::cout << "Moving Backward: " << mycameraPosition.v[2] << std::endl;
        break;
    case 'a': // Move left
        mycameraPosition.v[0] -= mymovementSpeed;
        std::cout << "Moving Left: " << mycameraPosition.v[0] << std::endl;
        break;
    case 'd': // Move right
        mycameraPosition.v[0] += mymovementSpeed;
        std::cout << "Moving Right: " << mycameraPosition.v[0] << std::endl;
        break;
    case 'q': // Rotate left (yaw)
        mycameraRotationY -= myrotationSpeed;
        std::cout << "Rotating Left: " << mycameraRotationY << std::endl;
        break;
    case 'e': // Rotate right (yaw)
        mycameraRotationY += myrotationSpeed;
        std::cout << "Rotating Right: " << mycameraRotationY << std::endl;
        break;
    case 'r': // Move up
        mycameraPosition.v[1] += mymovementSpeed;
        std::cout << "Moving Up: " << mycameraPosition.v[1] << std::endl;
        break;
    case 'f': // Move down
        mycameraPosition.v[1] -= mymovementSpeed;
        std::cout << "Moving Down: " << mycameraPosition.v[1] << std::endl;
        break;
    }
    glutPostRedisplay(); // Request a redraw to update the display with changes
}

void Keyboard::mouseButton(int button, int state, int x, int y) {
    std::cout << button << std::endl;
    if (button == GLUT_LEFT_BUTTON) {
        myleftMousePressed = (state == GLUT_DOWN);
        mylastMouseX = x;
        mylastMouseY = y;
    }
    else if (button == 3) { // Scroll up
        mycameraDistance -= 1.0f;
        if (mycameraDistance < 2.0f) mycameraDistance = 2.0f; // Prevent too close zoom
    }
    else if (button == 4) { // Scroll down
        mycameraDistance += 1.0f;
        if (mycameraDistance > 50.0f) mycameraDistance = 50.0f; // Limit zoom-out distance
    }
}

void Keyboard::mouseMotion(int x, int y) {
    if (myleftMousePressed) {
        float dx = x - mylastMouseX;
        float dy = y - mylastMouseY;
        mycameraRotationY += dx * 0.2f;
        mycameraRotationX += dy * 0.2f;
        if (mycameraRotationX > 89.0f) mycameraRotationX = 89.0f; // Prevent flipping over top
        if (mycameraRotationX < -89.0f) mycameraRotationX = -89.0f; // Prevent flipping over bottom
        mylastMouseX = x;
        mylastMouseY = y;
        glutPostRedisplay();
    }
}

// Setters for camera position, rotation, and distance
void Keyboard::setCameraPosition(float x, float y, float z) {
    mycameraPosition = vec3(x, y, z);
}

void Keyboard::setCameraRotation(float yaw, float pitch) {
    mycameraRotationY = yaw;
    mycameraRotationX = pitch;
}

void Keyboard::setCameraDistance(float distance) {
    mycameraDistance = distance;
}
