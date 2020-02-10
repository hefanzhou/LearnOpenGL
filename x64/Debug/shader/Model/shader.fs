#version 330 core
out vec4 FragColor;  
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
float specularStrength = 2;
float ambient = 0.2;

float near = 0.1; 
float far  = 100.0; 

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}


void main()
{

	vec3 textureColor = (texture(texture_diffuse1, TexCoord)).rgb;
   	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = specularStrength * spec * lightColor * vec3(texture(texture_specular1, TexCoord));
	vec3 result = (ambient + diff + specular) * textureColor;
	// vec3 result = (specular) * textureColor;

	FragColor = vec4(result, 1.0);
	 // FragColor = vec4(vec3(LinearizeDepth(gl_FragCoord.z)/far), 1.0);
}