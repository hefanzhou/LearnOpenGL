#version 330 core
layout (location = 0) in vec3 aPos;   // 位置变量的属性位置值为 0 
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
uniform sampler2D texture_diffuse;
uniform mat4 transformMVP;



vec2 EncodeFloatRG( float v )
{
    vec2 kEncodeMul = vec2(1.0, 255.0);
    float kEncodeBit = 1.0/255.0;
    vec2 enc = kEncodeMul * v;
    enc = fract (enc);
    enc.x -= enc.y * kEncodeBit;
    return enc;
}
float DecodeFloatRG( vec2 enc )
{
    vec2 kDecodeDot = vec2(1.0, 1/255.0);
    return dot( enc, kDecodeDot );
}




float DecodeHeight(vec4 rgba) {
   float d1 = DecodeFloatRG(rgba.rg);
   float d2 = DecodeFloatRG(rgba.ba);

   if (d1 >= d2)
      return d1;
   else
      return -d2;
}


vec4 EncodeHeight(float height) {
   vec2 rg = EncodeFloatRG(height >= 0 ? height : 0);
   vec2 ba = EncodeFloatRG(height < 0 ? -height : 0);

   return vec4(rg, ba);
}

out vec2 TexCoord;
void main()
{
    float height = DecodeHeight(texture(texture_diffuse, aTexCoord));
    vec3 pos = aPos;
    pos.y += height*5.;
    gl_Position = transformMVP*vec4(pos, 1);
    TexCoord = aTexCoord;
}