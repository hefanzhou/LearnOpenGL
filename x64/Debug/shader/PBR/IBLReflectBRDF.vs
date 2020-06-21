#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;
uniform mat4 projection;
uniform mat4 view;

out vec2 TexCoords;
void main()
{
   gl_Position = vec4(aPos.x, aPos.z, 1, 1);
   TexCoords = gl_Position.xy*0.5 + 0.5;
}