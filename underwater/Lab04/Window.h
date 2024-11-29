
#ifndef WINDOW_H
#define WINDOW_H

#include <string>
#include <iostream>

class Window {
public:
    // 构造函数
    //Window(const std::string& name, int value);

    void init(int argc, char** argv);


    // 私有成员变量
    std::string name;
    int value;
};

void display();


#endif
