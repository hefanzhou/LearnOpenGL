#version 330 core
out vec4 FragColor;  
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
uniform sampler2D texture_diffuse;

void main()
{

	vec3 textureColor = (texture(texture_diffuse, TexCoord)).rgb;

	FragColor = vec4(textureColor, 1.0);
}