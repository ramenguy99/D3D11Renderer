#include "defines.hlsl"

struct vertex_input
{
    vec3 Position : POSITION;
    vec2 TexCoord: TEXCOORD0;
};

struct pixel_input
{
    vec4 Position : SV_POSITION;
    vec2 TexCoord : TEXCOORD0;
};

//Vertex
pixel_input VertexMain(vertex_input In)
{
    pixel_input Out;
    
    Out.Position = vec4(In.Position, 1.0f);
    Out.TexCoord = In.TexCoord;
    
    return Out;
}
