#version 330 core
uniform sampler2D diffuseTexture;
in vec2 TexCoord;
out vec4 FragColor; 
void main()
{
    vec3 textureColor = (texture(diffuseTexture, TexCoord)).rgb;
    FragColor = vec4(textureColor, 1);
}