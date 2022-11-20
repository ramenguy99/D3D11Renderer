#include "atmosphere_common.hlsl"

struct vertex_input
{
    uint VertexId   : SV_VertexID;
    uint InstanceId : SV_InstanceID;
};

struct vertex_output
{
    vec4 Position : SV_POSITION;
    nointerpolation uint SliceId : SLICEINDEX;
};

struct geometry_output
{
    vec4 Position : SV_POSITION;
    nointerpolation uint SliceId : SV_RenderTargetArrayIndex; //write to a specific slice, it can also be read in the pixel shader.
};

vertex_output VertexMain(vertex_input In)
{
    vertex_output Out;
    
	// For a range on screen in [-0.5,0.5]
	vec2 UV = -1.0f;
	UV = In.VertexId == 1 ? vec2(-1.0f, 3.0f) : UV;
	UV = In.VertexId == 2 ? vec2( 3.0f,-1.0f) : UV;
	Out.Position = vec4(UV, 0.0f, 1.0f);
	Out.SliceId = In.InstanceId;
    
	return Out;
}

[maxvertexcount(3)]
void GeometryMain(triangle vertex_output Input[3], inout TriangleStream<geometry_output> GSOut)
{
    geometry_output Output;
    for (uint i = 0; i < 3; i++)
    {
        Output.Position = Input[i].Position;
        Output.SliceId = Input[0].SliceId;
        GSOut.Append(Output);
    }
    GSOut.RestartStrip();
}