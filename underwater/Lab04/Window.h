
#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <iostream>

class Window {
public:
    // ¹¹Ôìº¯Êý
    //Window(const std::string& name, int value);

    void init(int argc, char** argv);


    static const int width = 800;
    static const int height = 600;
};

void display();


#endif
