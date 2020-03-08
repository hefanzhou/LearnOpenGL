#version 330 core
out vec4 FragColor;  
uniform samplerCube textureShadow;
uniform sampler2D texure_diffuse;
uniform float far_plane;
uniform vec3 LightPos;
in vec2 TexCoord;
in vec3 FragPos;

float ShadowCalculation()
{
    // Get vector between fragment position and light position
    vec3 fragToLight = FragPos - LightPos;
    // Use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(textureShadow, fragToLight).r;

    // It is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= far_plane;
    // Now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);

    // Now test for shadows
    float bias = 0.05; 
    float shadow = (currentDepth -  bias) > closestDepth ? 1.0 : 0.0;

    return shadow;
}

void main()
{

    vec3 textureColor = (texture(texure_diffuse, TexCoord)).rgb;
    float shadow = ShadowCalculation();
    textureColor *=(1-shadow);
    FragColor = vec4( textureColor, 1.0);
}