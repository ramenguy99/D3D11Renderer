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
    mat4 Projection;
    mat4 View;
};

pixel_input VertexMain(vertex_input In)
{
    pixel_input Out;
    
    Out.Position = mul(Projection, mul(View, vec4(In.Position, 1.0f)));
    Out.LocalPos = In.Position;
    
    return Out;
}

