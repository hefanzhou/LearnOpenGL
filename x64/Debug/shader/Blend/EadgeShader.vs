#version 330 core
layout (location = 0) in vec3 aPos;   // 位置变量的属性位置值为 0 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
uniform mat4 transformMVP;
uniform mat4 transformM;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
void main()
{
	vec3 pos = aPos + aNormal*0.1;
    gl_Position = transformMVP*vec4(pos, 1);
    TexCoord = aTexCoord;
    Normal = mat3(transpose(inverse(transformM))) * aNormal;
    FragPos = vec3(transformM*vec4(pos, 1));
}