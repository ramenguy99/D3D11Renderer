#include "defines.hlsl"

Texture3D<vec4> Texture;
SamplerState SampleType;

struct pixel_input
{
    vec4 Position : SV_POSITION;
    vec2 TexCoord : TEXCOORD0;
};

cbuffer pixel_constants
{
    float w;
};

vec4 PixelMain(pixel_input In) : SV_TARGET
{
    vec4 Color = Texture.Sample(SampleType, vec3(In.TexCoord, w));
    return Color;
}
    