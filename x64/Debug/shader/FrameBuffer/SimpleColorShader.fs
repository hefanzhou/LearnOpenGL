#version 330 core
out vec4 FragColor;  
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
uniform vec4 diffuseColor;

void main()
{

	FragColor = vec4(TexCoord.x, TexCoord.y, 0, 1);
}