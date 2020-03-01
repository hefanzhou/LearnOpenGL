#version 330 core
layout (location = 0) in vec3 aPos;   // 位置变量的属性位置值为 0 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform mat4 transformM;
void main()
{
    gl_Position = projection * view * transformM * vec4(aPos, 1.0);
}