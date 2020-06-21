#version 330 core
layout (location = 0) in vec3 aPos;   // 位置变量的属性位置值为 0 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 transformMVP;
uniform mat4 transformM;

out vec2 TexCoords;
out vec3 WorldPos;
out mat3 TBN;
void main()
{
    gl_Position = transformMVP*vec4(aPos, 1);
    TexCoords = aTexCoord;
    WorldPos = vec3(transformM*vec4(aPos, 1));
    vec3 T = normalize(vec3(transformM * vec4(tangent,   0.0)));
   	vec3 B = normalize(vec3(transformM * vec4(bitangent, 0.0)));
   	vec3 N = normalize(vec3(transformM * vec4(aNormal,    0.0)));
   	TBN = mat3(T, B, N);
}