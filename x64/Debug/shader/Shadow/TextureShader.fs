#version 330 core
out vec4 FragColor;  
in vec2 TexCoord;
in vec4 FragPosLightSpace;
uniform sampler2D texture_diffuse;
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
    // 取得当前片段在光源视角下的深度
    float currentDepth = projCoords.z;
    float bias = 0.005;
    currentDepth -=bias;
    // 检查当前片段是否在阴影中
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main()
{

	vec3 textureColor = (texture(texture_diffuse, TexCoord)).rgb;
	float shadow = ShadowCalculation(FragPosLightSpace);
	textureColor *=(1-shadow);
	FragColor = vec4(textureColor, 1.0);
}