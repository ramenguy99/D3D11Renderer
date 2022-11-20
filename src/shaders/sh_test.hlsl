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

float EvaluateSphericalHarmonics(vec3 p)
{
    const float Y00 = 0.282095;
    
    const float Y11  = 0.488603 * p.x;
    const float Y10  = 0.488603 * p.z;
    const float Y1n1 = 0.488603 * p.y;
    
    const float Y21  = 1.092548 * p.x * p.z;
    const float Y2n1 = 1.092548 * p.y * p.z;
    const float Y2n2 = 1.092548 * p.x * p.y;
    
    const float Y20  = 0.315392 * (3 * p.z * p.z - 1);
    const float Y22  = 0.546274 * (p.x * p.x - p.y * p.y);
    
    
    return Y22;
        /*
        Y00 + 
        Y11 + Y10 + Y1n1 + 
        Y21 + Y2n1 + Y2n2 + 
        Y20 + Y22;
*/
}

//Pixel
float4 PixelMain(pixel_input In) : SV_TARGET
{
    float sh = EvaluateSphericalHarmonics(normalize(In.LocalPos));
    
    if(sh > 0)
        return float4(0, sh, 0, 1);
    else
        return float4(0, 0, -sh, 1);
}
