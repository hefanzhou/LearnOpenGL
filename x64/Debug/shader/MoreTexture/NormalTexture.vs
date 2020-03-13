#version 330 core
layout (location = 0) in vec3 aPos;    
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 transformMVP;
uniform mat4 transformM;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec2 TexCoord;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;
void main()
{
    gl_Position = transformMVP*vec4(aPos, 1);
    TexCoord = aTexCoord;

    vec3 T = normalize(vec3(transformM * vec4(tangent,   0.0)));
   	vec3 B = normalize(vec3(transformM * vec4(bitangent, 0.0)));
   	vec3 N = normalize(vec3(transformM * vec4(aNormal,    0.0)));
   	mat3 TBN = mat3(T, B, N);

   	TangentLightPos = TBN * lightPos;
    TangentViewPos  = TBN * viewPos;
    TangentFragPos  = TBN * vec3(transformM * vec4(aPos, 1));
}