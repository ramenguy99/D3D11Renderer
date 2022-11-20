#include "atmosphere_common.hlsl"

Texture3D RScatteringTexture        : register(t0);
Texture3D MScatteringTexture        : register(t1);
Texture3D MultipleScatteringTexture : register(t2);
SamplerState Sampler;

struct pixel_input
{
    vec4 Position : SV_POSITION;
    vec2 TexCoord : TEXCOORD0;
};

struct pixel_output
{
    vec4 DeltaIrradiance : SV_TARGET0;
    vec4 Irradiance      : SV_TARGET1;
};

vec3 ComputeIndirectIrradiance(float r, float mu_s)
{
    const int SAMPLE_COUNT = INDIRECT_IRRADIANCE_SAMPLE_COUNT;
    
    const float dphi = PI / float(SAMPLE_COUNT);
    const float dtheta = PI / float(SAMPLE_COUNT);
    
    vec3 result = vec3(0, 0, 0);
    vec3 omega_s = vec3(sqrt(1.0 - mu_s * mu_s), 0.0, mu_s);
    for (int j = 0; j < SAMPLE_COUNT / 2; ++j) {
        float theta = (float(j) + 0.5) * dtheta;
        for (int i = 0; i < 2 * SAMPLE_COUNT; ++i) {
            float phi = (float(i) + 0.5) * dphi;
            vec3 omega = vec3(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
            float domega = (dtheta) * (dphi) * sin(theta);
            
            float nu = dot(omega, omega_s);
            result += GetScattering(RScatteringTexture, MScatteringTexture, MultipleScatteringTexture, Sampler,
                                    r, omega.z, mu_s, nu, false /* ray_r_theta_intersects_ground */,
                                    ScatteringOrder) * omega.z * domega;
        }
    }
    return result;
}

pixel_output PixelMain(pixel_input In)
{
    pixel_output Out;
    float r, mu_s;
    GetRMuSFromIrradianceTextureUV(In.TexCoord, r, mu_s);
    vec3 IndirectIrradiance = ComputeIndirectIrradiance(r, mu_s);
    
    Out.DeltaIrradiance = vec4(IndirectIrradiance, 1.0f);
    Out.Irradiance = vec4(IndirectIrradiance, 1.0f);
    
    return Out;
}