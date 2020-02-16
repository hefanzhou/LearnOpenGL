#version 330 core
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;
uniform	vec2 iResolution;
uniform vec3 ripplePos;

out vec4 FragColor;  
in vec2 TexCoord;


// Encoding/decoding [0..1) floats into 8 bit/channel RG. Note that 1.0 will not be encoded properly.
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

void main()
{

   // return;
	vec3 e = vec3(0.00125, 0.00125, 0.);
   vec2 q = TexCoord;
   vec2 fragPos = q*vec2(800, 800);
   
   vec4 c = texture(iChannel0, q);
   
   float p11 = DecodeHeight(c);
   
   float p10 = DecodeHeight(texture(iChannel1, q-e.zy));
   float p01 = DecodeHeight(texture(iChannel1, q-e.xz));
   float p21 = DecodeHeight(texture(iChannel1, q+e.xz));
   float p12 = DecodeHeight(texture(iChannel1, q+e.zy));
   float p00 = DecodeHeight(texture(iChannel1, q));
   

   float d = 0.;
    
   if (ripplePos.z > 0.) 
   {
      d = smoothstep(10.5,.5,length(ripplePos.xy - fragPos.xy))*5;
   }
   
   float k1 = 0;
   float k2 = -1;
   float k3 = 0.5;

   // The actual propagation:
   d += k1*p00 + k2*p11 + k3*(p01 + p21 + p10 + p12);
   d *= .99; // dampening
   FragColor = EncodeHeight(d);
}

