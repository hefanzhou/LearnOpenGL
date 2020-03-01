#version 330 core
layout (location = 0) in vec3 aPos;   // 位置变量的属性位置值为 0 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
uniform mat4 transformMVP;
uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;

out VS_OUT {
    vec3 normal;
} vs_out;

void main()
{
    gl_Position = transformMVP*vec4(aPos, 1);
    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    vs_out.normal = normalize(vec3(projectionMatrix * vec4(normalMatrix * aNormal, 0.0)));
}