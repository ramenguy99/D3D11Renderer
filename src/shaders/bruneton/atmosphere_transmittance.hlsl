#include "atmosphere_common.hlsl"

struct pixel_input
{
    vec4 Position : SV_POSITION;
    vec2 TexCoord : TEXCOORD0;
};

//Map UV to r and mu
void GetRMuFromTransmittanceTextureUV(vec2 UV, out float r, out float mu) 
{
    float x_mu = GetUnitRangeFromTextureCoord(UV.x, TRANSMITTANCE_TEXTURE_WIDTH);
    float x_r = GetUnitRangeFromTextureCoord(UV.y, TRANSMITTANCE_TEXTURE_HEIGHT);
    // Distance to top atmosphere boundary for a horizontal ray at ground level.
    float H = sqrt(Rt * Rt - Rg * Rg);
    // Distance to the horizon, from which we can compute r:
    float rho = H * x_r;
    r = sqrt(rho * rho + Rg * Rg);
    // Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
    // and maximum values over all mu - obtained for (r,1) and (r,mu_horizon) -
    // from which we can recover mu:
    float d_min = Rt - r;
    float d_max = rho + H;
    float d = d_min + x_mu * (d_max - d_min);
    mu = d == 0.0 ? 1.0 : (H * H - rho * rho - d * d) / (2.0 * r * d);
    mu = clamp(mu, -1.0, 1.0);
}

//Computes the integral of atmospheric density over the path from a point
//at altitude r and the intersection with the top of the atmosphere 
//with cosine of the zenith angle mu.
vec3 ComputeOpticalDepthOverPath(float r, float mu, density_layer Layer0, density_layer Layer1, float LayerHeight)
{
    const int SAMPLE_COUNT = TRANSMITTANCE_SAMPLE_COUNT;
    
    // Number of intervals for the numerical integration.
    // The integration step, i.e. the length of each integration interval.
    float dx = DistanceToTopAtmosphereBoundary(r, mu) / SAMPLE_COUNT;
    // Integration loop.
    float result = 0.0;
    for (int i = 0; i <= SAMPLE_COUNT; ++i) {
        float d_i = float(i) * dx;
        // Distance between the current sample point and the planet center.
        float r_i = sqrt(d_i * d_i + 2.0 * r * mu * d_i + r * r);
        // Number density at the current sample point (divided by the number density
        // at the bottom of the atmosphere, yielding a dimensionless number).
        float y_i = GetDensity(r_i - Rg, Layer0, Layer1, LayerHeight);
        // Sample weight (from the trapezoidal rule).
        float weight_i = i == 0 || i == SAMPLE_COUNT ? 0.5 : 1.0;
        result += y_i * weight_i * dx;
    }
    return result;
}

//Computes the extinction factor along a path considering Rayleigh and Mie scattering
vec3 ComputeTransmittance(float r, float mu)
{
    vec3 t = 
        Ber * ComputeOpticalDepthOverPath(r, mu, RLayer, RLayer, 0.0f) + 
        Bem * ComputeOpticalDepthOverPath(r, mu, MLayer, MLayer, 0.0f) +
        Beo * ComputeOpticalDepthOverPath(r, mu, OLayers[0], OLayers[1], OLayerHeight);
    
    return exp(-t);
}

vec4 PixelMain(pixel_input In) : SV_TARGET
{
    float r, mu;
    GetRMuFromTransmittanceTextureUV(In.TexCoord, r, mu);
    vec3 Transmittance = ComputeTransmittance(r, mu);
    
    return vec4(Transmittance, 1.0);    
}

