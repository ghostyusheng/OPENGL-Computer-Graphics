#version 330 core

layout(location = 0) in vec3 vertex_position; // ���붥��λ��
out vec4 FragColor; // �����Ƭ����ɫ������ɫ

uniform mat4 view; // ��ͼ����
uniform mat4 proj; // ͶӰ����
uniform vec4 diffuseColor; // �������������ɫ

void main() {
    // ������λ��ת�����ü��ռ�
    gl_Position = proj * view * vec4(vertex_position, 1.0);
    FragColor = diffuseColor; // ����ɫ���ݸ�Ƭ����ɫ��
}
