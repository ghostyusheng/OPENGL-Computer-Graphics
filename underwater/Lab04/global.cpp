#include "global.h"

// ȫ�ֱ�������
std::vector<Model> objs;

vec3 mycameraPosition(0.0f, 0.0f, 10.0f);
float mycameraRotationY = 0.0f;     // Y����ת
float mycameraRotationX = 0.0f;     // ��ֱ��ת
float mycameraDistance = 10.0f;     // ������룬�������ſ���

float mymovementSpeed = 0.5f;       // ����ƶ��ٶ�
float myrotationSpeed = 5.0f;       // �����ת�ٶ�

bool myleftMousePressed = false;     // ������״̬
int mylastMouseX = 0;                // ��һ����� X ����
int mylastMouseY = 0;                // ��һ����� Y ����
