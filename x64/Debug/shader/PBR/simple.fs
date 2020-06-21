#version 330 core
layout (location = 0) out vec4 FragColor;
uniform sampler2D diffsuleTexture;
in vec2 TexCoord;
void main()
{
	vec3 textureColor = (texture(diffsuleTexture, TexCoord)).rgb;
	FragColor = vec4(textureColor, 1);
}
