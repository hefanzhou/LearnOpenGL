#version 330 core
out vec4 FragColor;  
in vec2 TexCoord;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

uniform sampler2D diffsuleTexture;
uniform sampler2D normalTexture;
uniform sampler2D depthMap;


float specularStrength = 2;
float ambient = 0.1;
float height_scale = 0.1;
vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
	// number of depth layers
    const float minLayers = 5;
	const float maxLayers = 10;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));

    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy * height_scale; 
    vec2 deltaTexCoords = P / numLayers;

   	vec2  currentTexCoords     = texCoords;
	float currentDepthMapValue = texture(depthMap, currentTexCoords).r;
	while(currentLayerDepth < currentDepthMapValue)
	{
	    // shift texture coordinates along direction of P
	    currentTexCoords -= deltaTexCoords;
	    // get depthmap value at current texture coordinates
	    currentDepthMapValue = texture(depthMap, currentTexCoords).r;  
	    // get depth of next layer
	    currentLayerDepth += layerDepth;  
	}
	vec2 preTexCoords = currentTexCoords + deltaTexCoords;
	float curDepthDiff = currentLayerDepth - currentDepthMapValue;
	float preDepthDiff = texture(depthMap, preTexCoords).r - (currentLayerDepth-layerDepth);

	float curLayerWidget = 1-curDepthDiff/(curDepthDiff + preDepthDiff);
	return currentTexCoords* curLayerWidget + preTexCoords*(1-curLayerWidget);
}

void main()
{
	vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
	vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
	
	vec2 texCoord = ParallaxMapping(TexCoord, viewDir);
	if(texCoord.x > 1.0 || texCoord.y > 1.0 || texCoord.x < 0.0 || texCoord.y < 0.0) {discard;}

	vec3 textureColor = (texture(diffsuleTexture, texCoord)).rgb;
   	vec3 normal = texture(normalTexture, texCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0);   

	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	float specular = specularStrength * spec;
	vec3 result = (ambient + diff + specular) * textureColor;

	FragColor = vec4(result, 1.0);
}