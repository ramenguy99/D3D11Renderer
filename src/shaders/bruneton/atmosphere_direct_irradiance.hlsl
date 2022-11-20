#include "atmosphere_common.hlsl"

Texture2D TransmittanceTexture;
SamplerState Sampler;

struct pixel_input
{
    vec4 Position : SV_POSITION;
    vec2 TexCoord : TEXCOORD0;
};

vec3 ComputeDirectIrradiance(float r, float mu_s)
{
    float alpha_s = SunAngularRadius;
    
    // Approximate average of the cosine factor mu_s over the visible fraction of the sun disc.
    float average_cosine_factor = mu_s < -alpha_s ? 0.0 : (mu_s > alpha_s ? mu_s : (mu_s + alpha_s) * (mu_s + alpha_s) / (4.0 * alpha_s));
    vec3 transmittance = GetTransmittanceToTopAtmosphere(TransmittanceTexture, Sampler, r, mu_s);
    
    return SolarIrradiance * transmittance  * average_cosine_factor;
}

vec4 PixelMain(pixel_input In) : SV_TARGET
{
    float r, mu_s;
    GetRMuSFromIrradianceTextureUV(In.TexCoord, r, mu_s);
    vec3 Irradiance = ComputeDirectIrradiance(r, mu_s);
    
    vec4 Result = vec4(Irradiance, 1.0f); 
    return Result;
}
