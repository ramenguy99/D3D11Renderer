#include "defines.hlsl"

Texture2D SourceBuffer : register(t0);

struct pixel_input
{
    vec4 Position : SV_POSITION;
    vec2 TexCoord : TEXCOORD0;
};

float sRGB(float x)
{
    if (x <= 0.00031308)
        return 12.92 * x;
    else
        return 1.055*pow(x, (1.0 / 2.4)) - 0.055;
}

float4 sRGB(float4 vec)
{
    return float4(sRGB(vec.x), sRGB(vec.y), sRGB(vec.z), vec.w);
}

float3 sRGB(float3 vec)
{
    return float3(sRGB(vec.x), sRGB(vec.y), sRGB(vec.z));
}

vec4 PixelMain(pixel_input In) : SV_TARGET
{
    vec4 Source = SourceBuffer[In.Position.xy];
    
    vec3 Color = Source.xyz;
    
#if 0
    float Exposure = 1.0;
	vec3 WhitePoint = float3(1.08241, 0.96756, 0.95003);
    Color = 1.0 - exp(-Color / WhitePoint * Exposure);
#else
    Color = Color / (Color + vec3(1.0f, 1.0f, 1.0f));
#endif
    
    Color = sRGB(Color);
    return vec4(Color, 1.0f);
}