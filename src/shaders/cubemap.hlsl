#include "defines.hlsl"

struct vertex_input
{
    vec3 Position : POSITION;
};

struct pixel_input
{
    vec4 Position : SV_POSITION;
    vec3 LocalPos : POSITION;
};

cbuffer vertex_constants
{
    matrix Projection;
    matrix View;
    matrix Model;
};

//Vertex
pixel_input VertexMain(vertex_input In)
{
    pixel_input Out;
    
    Out.Position = mul(Projection, mul(View, mul(Model, vec4(In.Position, 1.0f))));
    Out.LocalPos = In.Position;
    
    return Out;
}

//Pixel
cbuffer pixel_constants
{
    uint MipLevel;
};

TextureCube CubeMap;
SamplerState SampleType;

float4 PixelMain(pixel_input In) : SV_TARGET
{
    vec3 Color = CubeMap.SampleLevel(SampleType, In.LocalPos, MipLevel).rgb;
    
    // Reinhard tone mapping
    Color = Color / (Color + vec3(1.0f, 1.0f, 1.0f));
    
    return float4(Color, 1.0f);
}
