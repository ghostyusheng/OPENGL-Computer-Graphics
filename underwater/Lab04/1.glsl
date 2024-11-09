#version 330 core

layout(location = 0) in vec3 vertex_position; // 输入顶点位置
out vec4 FragColor; // 输出到片段着色器的颜色

uniform mat4 view; // 视图矩阵
uniform mat4 proj; // 投影矩阵
uniform vec4 diffuseColor; // 从主程序传入的颜色

void main() {
    // 将顶点位置转换到裁剪空间
    gl_Position = proj * view * vec4(vertex_position, 1.0);
    FragColor = diffuseColor; // 将颜色传递给片段着色器
}
