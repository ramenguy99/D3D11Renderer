#include "defines.hlsl"

struct pixel_input
{
    vec4 Position : SV_POSITION;
    vec3 LocalPos : POSITION;
};

cbuffer pixel_constants
{
    vec4 Color;
};

TextureCube<vec4> environmentMap;
SamplerState SampleType;

vec4 PixelMain(pixel_input In) : SV_TARGET
{
    // The world vector acts as the normal of a tangent surface
    // from the origin, aligned to WorldPos. Given this normal, calculate all
    // incoming radiance of the environment. The result of this radiance
    // is the radiance of light coming from -Normal direction, which is what
    // we use in the PBR shader to sample irradiance.
    vec3 N = normalize(In.LocalPos);
    N.x = -N.x;

    vec3 irradiance = vec3(0.0f, 0.0f, 0.0f);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0f, 1.0f, 0.0f);
    vec3 right = cross(up, N);
    up         = cross(N, right);
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
        
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += environmentMap.SampleLevel(SampleType, sampleVec, 0).rgb * cos(theta) * sin(theta);
            nrSamples += 1.0f;
        }
    }
    
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    return vec4(irradiance, 1.0f);
}


