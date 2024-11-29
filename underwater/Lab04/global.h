#pragma once

#include "Mesh.h"
#include <vector>

// 全局变量声明
extern std::vector<Model> objs;

extern vec3 mycameraPosition;      // 相机位置
extern float mycameraRotationY;    // Y轴旋转
extern float mycameraRotationX;    // 垂直旋转
extern float mycameraDistance;      // 相机距离，用于缩放控制

extern float mymovementSpeed;       // 相机移动速度
extern float myrotationSpeed;       // 相机旋转速度

extern bool myleftMousePressed;     // 鼠标左键状态
extern int mylastMouseX;            // 上一次鼠标 X 坐标
extern int mylastMouseY;            // 上一次鼠标 Y 坐标
