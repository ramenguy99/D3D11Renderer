#include "defines.hlsl"
#include "pbr_common.hlsl"

struct vertex_input
{
    vec3 Position : POSITION;
    vec3 Normal : NORMAL;
    vec3 Tangent : TANGENT;
    vec2 TexCoord: TEXCOORD0;
};

//Vertex
cbuffer vertex_constants
{
    mat4 Projection;
    mat4 View;
    mat4 Model;
    mat4 NormalMatrix;
    mat4 ShadowMatrix[MAX_DIRECTIONAL_LIGHTS_COUNT];
};


pixel_input VertexMain(vertex_input In)
{
    pixel_input Out;
    
    float4 WorldPos = mul(Model, float4(In.Position, 1.0f));
    Out.Position = mul(Projection, mul(View, WorldPos));
    Out.WorldPos = WorldPos.xyz;
    Out.TexCoord = In.TexCoord;
    
    Out.Normal = mul(NormalMatrix, float4(In.Normal, 0.0f)).xyz;
    Out.Tangent = mul(NormalMatrix, float4(In.Tangent, 0.0f)).xyz;
    for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS_COUNT; i++)
    {
        Out.ShadowPos[i] = mul(ShadowMatrix[i], float4(WorldPos.xyz, 1.0f));
    }
    
    return Out;
}




