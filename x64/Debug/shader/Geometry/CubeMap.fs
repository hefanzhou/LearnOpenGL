#version 330 core
uniform samplerCube texture_diffuse;
in vec3 TexCoord;
out vec4 FragColor;  

void main()
{

	vec3 textureColor = (texture(texture_diffuse, TexCoord)).rgb;

	FragColor = vec4(textureColor, 1.0);
}