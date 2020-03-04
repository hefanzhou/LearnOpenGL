#version 330 core
out vec4 FragColor;  
in vec4 FragPosLightSpace;
uniform sampler2D textureShadow;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(projCoords.z > 1.0)
        return 0.0;
    // 变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;
    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = texture(textureShadow, projCoords.xy).r; 
 	float currentDepth = projCoords.z;
    
    float shadow = 0.0;
    float bias = 0.005;
	vec2 texelSize = 1.0 / textureSize(textureShadow, 0);
	for(int x = -1; x <= 1; ++x)
	{
	    for(int y = -1; y <= 1; ++y)
	    {
	        float pcfDepth = texture(textureShadow, projCoords.xy + vec2(x, y) * texelSize).r; 
	        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
	    }    
	}
	shadow /= 9.0;
    return shadow;
}

void main()
{

	vec3 textureColor = vec3(0.8);
	float shadow = ShadowCalculation(FragPosLightSpace);
	textureColor *=(1-shadow);
	FragColor = vec4(textureColor, 1.0);
}