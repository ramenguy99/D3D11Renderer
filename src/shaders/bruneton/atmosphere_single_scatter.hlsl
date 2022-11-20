#include "atmosphere_common.hlsl"

struct geometry_output
{
    vec4 Position : SV_POSITION;
    nointerpolation uint SliceId : SV_RenderTargetArrayIndex;
};

Texture2D  TransmittanceTexture : register(t0);
SamplerState Sampler;

struct pixel_output
{
    vec4 DeltaRayleigh: SV_TARGET0;
    vec4 DeltaMie:     SV_TARGET1;
    vec4 Scattering:   SV_TARGET2;
};

void ComputeSingleScatteringIntegrand(Texture2D TransmittanceTexture, 
                                      float r, float mu, float mu_s, float nu, float d,  bool IntersectsGround,
                                      out vec3 rayleigh, out vec3 mie)
{
    float r_d = ClampRadius(sqrt(d * d + 2.0 * r * mu * d + r * r));
    float mu_s_d = ClampCosine((r * mu_s + d * nu) / r_d);
    vec3 transmittance = GetTransmittance(TransmittanceTexture, Sampler, r, mu, d, IntersectsGround) *
        GetTransmittanceToSun(TransmittanceTexture, Sampler, r_d, mu_s_d);
    float h = r_d - Rg;
    rayleigh = transmittance * GetDensityAtLayer(h, RLayer);
    mie = transmittance * GetDensityAtLayer(h, MLayer);
}

void ComputeSingleScattering(Texture2D TransmittanceTexture, 
                             float r, float mu, float mu_s, float nu, bool IntersectsGround,
                             out vec3 rayleigh, out vec3 mie)
{
    const int SAMPLE_COUNT = SINGLE_SCATTERING_SAMPLE_COUNT;
    
    // The integration step, i.e. the length of each integration interval.
    float dx = DistanceToNearestAtmosphereBoundary(r, mu,  IntersectsGround) / (float)SAMPLE_COUNT;
    
    // Integration loop.
    vec3 rayleigh_sum = vec3(0.0, 0.0, 0.0);
    vec3 mie_sum = vec3(0.0, 0.0, 0.0);
    
    for (int i = 0; i <= SAMPLE_COUNT; ++i) 
    {
        float d_i = float(i) * dx;
        // The Rayleigh and Mie single scattering at the current sample point.
        vec3 rayleigh_i;
        vec3 mie_i;
        ComputeSingleScatteringIntegrand(TransmittanceTexture, r, mu, mu_s, nu, d_i, IntersectsGround, rayleigh_i, mie_i);
        
        // Sample weight (from the trapezoidal rule).
        float weight_i = (i == 0 || i == SAMPLE_COUNT) ? 0.5 : 1.0;
        rayleigh_sum += rayleigh_i * weight_i;
        mie_sum += mie_i * weight_i;
    }
    rayleigh = rayleigh_sum * dx * SolarIrradiance * Bsr;
    mie = mie_sum * dx * SolarIrradiance * Bsm;
}

pixel_output PixelMain(geometry_output In)
{
    pixel_output Out;
    
    vec3 FragCoords = vec3(In.Position.xy, In.SliceId + 0.5f);
    float r, mu, mu_s, nu;
    bool IntersectsGround;
    GetRMuMuSNuFromFragCoords(FragCoords, r, mu, mu_s, nu, IntersectsGround);
    
    vec3 DeltaRayleigh, DeltaMie;
    ComputeSingleScattering(TransmittanceTexture, r, mu, mu_s, nu, IntersectsGround, DeltaRayleigh, DeltaMie);
    Out.DeltaRayleigh = vec4(DeltaRayleigh, 1.0f);
    Out.DeltaMie      = vec4(DeltaMie, 1.0f);
    Out.Scattering    = vec4(DeltaRayleigh, DeltaMie.r);
    
    return Out;
}