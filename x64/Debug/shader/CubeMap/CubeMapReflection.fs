#version 330 core
uniform samplerCube texture_diffuse;
uniform vec3 viewPos;
in vec2 TexCoord;
in vec3 fragWorldPos;
in vec3 Normal;

out vec4 FragColor;  
void main()
{
	vec3 reflectDir = reflect(fragWorldPos - viewPos, Normal);
	vec3 textureColor = (texture(texture_diffuse, reflectDir)).rgb;

	FragColor = vec4(textureColor, 1.0);
}