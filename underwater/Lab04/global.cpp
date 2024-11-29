#include "global.h"

// 全局变量定义
std::vector<Model> objs;

vec3 mycameraPosition(0.0f, 0.0f, 10.0f);
float mycameraRotationY = 0.0f;     // Y轴旋转
float mycameraRotationX = 0.0f;     // 垂直旋转
float mycameraDistance = 10.0f;     // 相机距离，用于缩放控制

float mymovementSpeed = 0.5f;       // 相机移动速度
float myrotationSpeed = 5.0f;       // 相机旋转速度

bool myleftMousePressed = false;     // 鼠标左键状态
int mylastMouseX = 0;                // 上一次鼠标 X 坐标
int mylastMouseY = 0;                // 上一次鼠标 Y 坐标
