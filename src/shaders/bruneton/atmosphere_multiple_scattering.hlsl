#include "atmosphere_common.hlsl"

struct geometry_output
{
    vec4 Position : SV_POSITION;
    nointerpolation uint SliceId : SV_RenderTargetArrayIndex;
};

Texture2D  TransmittanceTexture    : register(t0);
Texture3D ScatteringDensityTexture : register(t1);
SamplerState Sampler;

struct pixel_output
{
    vec4 DeltaMultiple:     SV_TARGET0;
    vec4 Scattering:        SV_TARGET1;
};

vec3 ComputeMultipleScattering(float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground)
{
    const int SAMPLE_COUNT = MULTIPLE_SCATTERING_SAMPLE_COUNT;
    
    // Number of intervals for the numerical integration.
    // The integration step, i.e. the length of each integration interval.
    float dx = DistanceToNearestAtmosphereBoundary(r, mu, ray_r_mu_intersects_ground) / float(SAMPLE_COUNT);
    // Integration loop.
    vec3 rayleigh_mie_sum = 0.0;
    
    for (int i = 0; i <= SAMPLE_COUNT; ++i) {
        float d_i = float(i) * dx;
        
        // The r, mu and mu_s parameters at the current integration point (see the
        // single scattering section for a detailed explanation).
        float r_i = ClampRadius(sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r));
        float mu_i = ClampCosine((r * mu + d_i) / r_i);
        float mu_s_i = ClampCosine((r * mu_s + d_i * nu) / r_i);
        
        // The Rayleigh and Mie multiple scattering at the current sample point.
        vec3 rayleigh_mie_i =
            GetScattering(ScatteringDensityTexture, Sampler, r_i, mu_i, mu_s_i, nu, ray_r_mu_intersects_ground) *
            GetTransmittance(TransmittanceTexture, Sampler,  r, mu, d_i, ray_r_mu_intersects_ground)
            * dx;
        
        // Sample weight (from the trapezoidal rule).
        float weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
        rayleigh_mie_sum += rayleigh_mie_i * weight_i;
    }
    return rayleigh_mie_sum;
}

pixel_output PixelMain(geometry_output In)
{
    pixel_output Out;
    
    vec3 FragCoords = vec3(In.Position.xy, In.SliceId + 0.5f);
    float r, mu, mu_s, nu;
    bool IntersectsGround;
    GetRMuMuSNuFromFragCoords(FragCoords, r, mu, mu_s, nu, IntersectsGround);
    
    vec3 DeltaMultiple = ComputeMultipleScattering(r, mu, mu_s, nu, IntersectsGround);
    Out.DeltaMultiple = vec4(DeltaMultiple, 1.0f);
    Out.Scattering = vec4(DeltaMultiple / RayleighPhaseFunction(nu), 0.0f);
    
    return Out;
}
    