#version 330 core

in vec4 FragColor; // 从顶点着色器接收颜色
out vec4 finalColor; // 输出颜色

void main() {
    finalColor = FragColor; // 直接输出颜色
}
