
#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <iostream>

class Window {
public:
    // ���캯��
    //Window(const std::string& name, int value);

    void init(int argc, char** argv);


    // ˽�г�Ա����
    std::string name;
    int value;
};

void display();


#endif
