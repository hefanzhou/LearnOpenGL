#version 330 core
out vec4 FragColor;  
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
uniform sampler2D ourTexture;
uniform sampler2D specularTexture;
uniform sampler2D emissionTexture;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
float specularStrength = 2;
float ambient = 0.1;
void main()
{

	vec3 textureColor = (texture(ourTexture, TexCoord)).rgb;
   	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0)*0.6;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = specularStrength * spec * lightColor * vec3(texture(specularTexture, TexCoord));
	vec3 emission = vec3(texture(emissionTexture, TexCoord));
	vec3 result = (ambient + diff + specular) * textureColor + emission;
	// vec3 result = (specular) * textureColor;

	FragColor = vec4(result, 1.0);
}