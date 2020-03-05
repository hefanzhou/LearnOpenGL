#version 330 core
layout (location = 0) in vec3 aPos;   // 位置变量的属性位置值为 0 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 instanceMatrix;

uniform mat4 transformVP;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
void main()
{
    gl_Position = transformVP*instanceMatrix*vec4(aPos, 1);
    TexCoord = aTexCoord;
}