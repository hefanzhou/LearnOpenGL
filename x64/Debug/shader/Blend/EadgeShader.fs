#version 330 core
out vec4 FragColor;  
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;


void main()
{

	vec3 textureColor = vec3(0, 0, 1);
   	vec3 norm = normalize(Normal);
	

	FragColor = vec4(textureColor, 1.0);
}