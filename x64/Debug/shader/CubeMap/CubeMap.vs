#version 330 core
layout (location = 0) in vec3 aPos;   // 位置变量的属性位置值为 0 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
uniform mat4 transformVP;

out vec3 TexCoord;
void main()
{
    gl_Position = transformVP*vec4(aPos, 1);
    gl_Position.z = gl_Position.w;
    TexCoord = aPos*2.0;
}