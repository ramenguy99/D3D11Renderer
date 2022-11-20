#include "defines.hlsl"

struct vertex_input
{
    vec3 Position : POSITION;
};

struct pixel_input
{
    vec4 Position : SV_POSITION;
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
    Out.Position = mul(LightMatrix, mul(Model, vec4(In.Position, 1.0f)));
    
    return Out;
}


