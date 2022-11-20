struct vertex_input
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct pixel_input
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

//Vertex
cbuffer vertex_constants
{
    matrix Model;
    matrix Camera;
};


pixel_input VertexMain(vertex_input In)
{
    pixel_input Out;
    
    float4 Position = float4(In.Position, 1.0f);
    Out.Position = mul(Camera, mul(Model, Position));
    Out.Color = In.Color;
    
    return Out;
}


//Pixel
float4 PixelMain(pixel_input In) : SV_TARGET
{
    return In.Color;
}

