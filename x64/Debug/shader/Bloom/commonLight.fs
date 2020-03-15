#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
uniform sampler2D diffuseTexture;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
void main()
{

	vec3 textureColor = (texture(diffuseTexture, TexCoord)).rgb;
   	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 result = (spec+diff) * textureColor * lightColor;
	FragColor = vec4(result, 1.0);

	BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
