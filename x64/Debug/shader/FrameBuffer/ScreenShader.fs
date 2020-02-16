#version 330 core
in vec2 TexCoord;
out vec4 FragColor;  


void main()
{
   FragColor = vec4(1, 1, 1, 1);
   // FragColor = vec4(TexCoord.x, TexCoord.y, 0, 1);
   
}

