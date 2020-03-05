#version 330 core
layout (location = 0) in vec3 aPos;   // 位置变量的属性位置值为 0 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
uniform mat4 transformMVP;
uniform mat4 lightSpaceMatrix;

out vec2 TexCoord;
out vec4 FragPosLightSpace;
void main()
{
    gl_Position = transformMVP*vec4(aPos, 1);
    FragPosLightSpace = lightSpaceMatrix*vec4(aPos, 1);
    TexCoord = aTexCoord;
}
