#version 330 core
out vec4 FragColor;  
in vec2 TexCoords;
in vec3 WorldPos;
in mat3 TBN;
uniform vec3 camPos;
uniform float ao;

uniform sampler2D albedoTexture;
uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D normalTexture;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D   brdfLUT;

uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}   


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main()
{
	vec3 albedo = texture(albedoTexture, TexCoords).rgb;
	float metallic = texture(metallicTexture, TexCoords).r;
	float roughness = texture(roughnessTexture, TexCoords).r;
	vec3 Normal = texture(normalTexture, TexCoords).rgb;
    Normal = Normal * 2.0 - 1.0;
    Normal = TBN * Normal;

	vec3 N = normalize(Normal); 
    vec3 V = normalize(camPos - WorldPos);
	vec3 F0 = vec3(0.04); 
    vec3 Lo = vec3(0.0);
    vec3 R = reflect(-V, N);
	F0 = mix(F0, albedo, metallic);
    
	for(int i = 0; i < 4; ++i) 
	{
	    vec3 L = normalize(lightPositions[i] - WorldPos);
	    vec3 H = normalize(V + L);

	    float distance    = length(lightPositions[i] - WorldPos);
	    float attenuation = 1.0 / (distance * distance);
	    vec3 radiance     = lightColors[i] * attenuation; 
		vec3 F  = fresnelSchlickRoughness(max(dot(H, V), 0.0), F0, roughness);

	    

		float NDF = DistributionGGX(N, H, roughness);       
		float G   = GeometrySmith(N, V, L, roughness);  

		vec3 nominator    = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; 
		vec3 specular     = nominator / denominator;      

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;

		kD *= 1.0 - metallic;  

		float NdotL = max(dot(N, L), 0.0);
    	Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}
	vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;     

	vec3 irradiance = texture(irradianceMap, N).rgb;
	vec3 diffuse    = irradiance * albedo;

	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness*MAX_REFLECTION_LOD).rgb;   
	vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
	vec3 ambient = (kD * diffuse + specular) * ao; 

	vec3 color   = ambient + Lo;  
	
	// color = color / (color + vec3(1.0));
	// color = pow(color, vec3(1.0/2.2));
	// color = ambient + Lo;
	FragColor = vec4(color, 1);
}