#include "defines.hlsl"

struct vertex_input
{
    vec3 Position : POSITION;
};

struct pixel_input
{
    vec4 Position : SV_POSITION;
    vec3 WorldPosition : POSITION0;
};

struct pixel_output
{
    float Depth : SV_Depth;
};

cbuffer vertex_constants
{
    mat4 LightMatrix;
    mat4 Model;
}

//Vertex
pixel_input VertexMain(vertex_input In)
{
    pixel_input Out;
    vec4 WorldPosition = mul(Model, vec4(In.Position, 1.0f));
    Out.Position = mul(LightMatrix, WorldPosition);
    Out.WorldPosition = WorldPosition.xyz;
    
    return Out;
}

cbuffer pixel_constants
{
    vec3 LightPosition;
    float FarPlane;
}

//Pixel
pixel_output PixelMain(pixel_input In)
{
    pixel_output Out;
    
    Out.Depth = length(In.WorldPosition - LightPosition) / FarPlane;
    
    return Out;
}
    
    