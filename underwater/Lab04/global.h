#pragma once

#include "Mesh.h"
#include <vector>

// ȫ�ֱ�������
extern std::vector<Model> objs;

extern vec3 mycameraPosition;      // ���λ��
extern float mycameraRotationY;    // Y����ת
extern float mycameraRotationX;    // ��ֱ��ת
extern float mycameraDistance;      // ������룬�������ſ���

extern float mymovementSpeed;       // ����ƶ��ٶ�
extern float myrotationSpeed;       // �����ת�ٶ�

extern bool myleftMousePressed;     // ������״̬
extern int mylastMouseX;            // ��һ����� X ����
extern int mylastMouseY;            // ��һ����� Y ����
