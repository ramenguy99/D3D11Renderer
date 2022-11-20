#include "defines.hlsl"

struct pixel_input
{
    vec4 Position : SV_POSITION;
    vec3 LocalPos : POSITION;
};

Texture2D Texture;
SamplerState SampleType;

float2 SampleSphericalMap(vec3 v)
{
    const float2 invAtan = float2(0.1591, 0.3183);
#ifdef Y_UP
    float2 uv = float2(atan2(v.z,-v.x), asin(v.y));
#else //Z_UP
    float2 uv = float2(atan2(v.y,-v.x), asin(v.z));
#endif
    uv *= invAtan;
    uv += 0.5;
    return uv;
}


vec4 PixelMain(pixel_input In) : SV_TARGET
{
    vec2 uv = SampleSphericalMap(normalize(In.LocalPos));
    uv.y = 1.0f - uv.y;
    vec3 Color = Texture.Sample(SampleType, uv);
    return vec4(Color, 1.0f);
}