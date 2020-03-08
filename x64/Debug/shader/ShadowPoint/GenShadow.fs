#version 330 core

uniform vec3 lightPos;
uniform float far_plane;
in vec4 FragPos;
void main()
{

	  float lightDistance = length(FragPos.xyz - lightPos);

    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / far_plane;

    // write this as modified depth
    gl_FragDepth = lightDistance;
}