#include "Render.h"

void Render::render()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // Set background color to a deep blue
    glClearColor(0.0f, 0.0f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Render::initResource()
{

}
