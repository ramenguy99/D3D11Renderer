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

cbuffer pixel_constants
{
    vec3 L[9];
};

// L00 L11 L10 L1n1 L22 L21 L20 L2n1 L2n2
//  0   1   2    3   4   5   6    7    8
vec3 EvaluateSphericalHarmonics(vec3 n)
{
    float x = n.x;
    float y = n.y;
    float z = n.z;
    
    const float c1 = 0.429043;
    const float c2 = 0.511664;
    const float c3 = 0.743125;
    const float c4 = 0.886227;
    const float c5 = 0.247708;
    
    vec3 E = 
        c1 * L[4] * (x * x - y * y) + c3 * L[6] * (z * z) + c4 * L[0] - c5 * L[6] +
        2 * c1 * (L[8] * x * y + L[5] * x * z + L[7] * y * z) +
        2 * c2 * (L[1] * x + L[3] * y + L[2] * z);
    
    return E;
}

//Pixel
float4 PixelMain(pixel_input In) : SV_TARGET
{
    vec3 n = normalize(In.LocalPos);
    vec3 Color = EvaluateSphericalHarmonics(n);
    Color = Color / (Color + vec3(1.0f, 1.0f, 1.0f));
    
    return vec4(Color, 1.0f);
}
