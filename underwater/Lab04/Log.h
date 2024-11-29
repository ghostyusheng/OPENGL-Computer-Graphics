#ifndef LOG_H
#define LOG_H

#include <windows.h>
#include <iostream>
#include <string>


class Log {
public:
    enum class Level {
        INFO,
        WARNING
    };

    static void log(Level level, const std::string& message) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        switch (level) {
        case Level::INFO:
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "INFO: " << message << std::endl;
            break;
        case Level::WARNING:
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "WARNING: " << message << std::endl;
            break;
        }
        // 重置颜色为默认
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
};

#endif // LOG_H
