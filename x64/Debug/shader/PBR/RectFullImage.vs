#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;
uniform mat4 transformMVP;

out vec3 locationPos;
void main()
{
    gl_Position = transformMVP*vec4(aPos, 1);
    locationPos = aPos;
}