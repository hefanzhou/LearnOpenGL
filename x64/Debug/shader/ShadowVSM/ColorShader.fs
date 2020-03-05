#version 330 core
out vec4 FragColor;  
in vec4 FragPosLightSpace;
in vec2 TexCoord;
uniform sampler2D textureShadow;

float chebyshevUpperBound(sampler2D shadowMap, float d, vec2 coord)
{
    vec2 moments = texture(shadowMap, coord).rg;
    // Surface is fully lit. as the current fragment is before the light occluder
    if (d <= moments.x)
        return 1.0;

    // The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
    // How likely this pixel is to be lit (p_max)
    float variance = moments.y - (moments.x * moments.x);
    //variance = max(variance, 0.000002);
    variance = max(variance, 0.00002);

    float d_minus_mean = d - moments.x;
    float p_max = variance / (variance + d_minus_mean * d_minus_mean);

    return p_max;
}

// 返回阴影百分比[0,1], 然后拿去乘以光照颜色即可
float ShadowCalculation(vec4 fragPosLightSpace) {

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;
    float shadow = 1 - chebyshevUpperBound(textureShadow, currentDepth, projCoords.xy);
    return shadow;
}

void main()
{

	vec3 textureColor = vec3(0.8);
	float shadow = ShadowCalculation(FragPosLightSpace);
	textureColor *=(1-shadow);
	FragColor = vec4(textureColor, 1.0);
}