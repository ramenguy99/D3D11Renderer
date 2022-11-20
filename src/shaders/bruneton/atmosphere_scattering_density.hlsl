#include "atmosphere_common.hlsl"

struct geometry_output
{
    vec4 Position : SV_POSITION;
    nointerpolation uint SliceId : SV_RenderTargetArrayIndex;
};

Texture2D TransmittanceTexture            : register(t0);
Texture2D IrradianceTexture               : register(t1);
Texture3D RScatteringTexture              : register(t2);
Texture3D MScatteringTexture              : register(t3);
Texture3D MultipleScatteringTexture       : register(t4);
SamplerState Sampler;

vec3 ComputeScatteringDensity(float r, float mu, float mu_s, float nu)
{
    // Compute unit direction vectors for the zenith, the view direction omega and
    // and the sun direction omega_s, such that the cosine of the view-zenith
    // angle is mu, the cosine of the sun-zenith angle is mu_s, and the cosine of
    // the view-sun angle is nu. The goal is to simplify computations below.
    vec3 zenith_direction = vec3(0.0, 0.0, 1.0);
    vec3 omega = vec3(sqrt(1.0 - mu * mu), 0.0, mu);
    float sun_dir_x = omega.x == 0.0 ? 0.0 : (nu - mu * mu_s) / omega.x;
    float sun_dir_y = sqrt(max(1.0 - sun_dir_x * sun_dir_x - mu_s * mu_s, 0.0));
    vec3 omega_s = vec3(sun_dir_x, sun_dir_y, mu_s);
    
    const int SAMPLE_COUNT = SCATTERING_DENSITY_SAMPLE_COUNT;
    
    const float dphi = PI / float(SAMPLE_COUNT);
    const float dtheta = PI / float(SAMPLE_COUNT);
    vec3 rayleigh_mie = 0.0;// vec3(0.0 * watt_per_cubic_meter_per_sr_per_nm);
    
    // Nested loops for the integral over all the incident directions omega_i.
    for (int l = 0; l < SAMPLE_COUNT; ++l) {
        float theta = (float(l) + 0.5) * dtheta;
        float cos_theta = cos(theta);
        float sin_theta = sin(theta);
        bool ray_r_theta_intersects_ground = RayIntersectsGround(r, cos_theta);
        
        // The distance and transmittance to the ground only depend on theta, so we
        // can compute them in the outer loop for efficiency.
        float distance_to_ground = 0.0;
        vec3 transmittance_to_ground = vec3(0.0, 0.0, 0.0);
        vec3 ground_albedo = vec3(0.0, 0.0, 0.0);
        if (ray_r_theta_intersects_ground) {
            distance_to_ground = DistanceToBottomAtmosphereBoundary(r, cos_theta);
            transmittance_to_ground = GetTransmittance(TransmittanceTexture, Sampler,  r, cos_theta, distance_to_ground, true /* ray_intersects_ground */);
            ground_albedo = GroundAlbedo;
        }
        
        for (int sample = 0; sample < 2 * SAMPLE_COUNT; ++sample) {
            float phi = (float(sample) + 0.5) * dphi;
            vec3 omega_i = vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
            float domega_i = dtheta * dphi * sin(theta);
            
            // The radiance L_i arriving from direction omega_i after n-1 bounces is
            // the sum of a term given by the precomputed scattering texture for the
            // (n-1)-th order:
            float nu1 = dot(omega_s, omega_i);
            vec3 incident_radiance = GetScattering(RScatteringTexture, MScatteringTexture, MultipleScatteringTexture, Sampler,
                                                   r, omega_i.z, mu_s, nu1,  ray_r_theta_intersects_ground, ScatteringOrder - 1);
            
            // and of the contribution from the light paths with n-1 bounces and whose
            // last bounce is on the ground. This contribution is the product of the
            // transmittance to the ground, the ground albedo, the ground BRDF, and
            // the irradiance received on the ground after n-2 bounces.
            vec3 ground_normal =  normalize(zenith_direction * r + omega_i * distance_to_ground);
            vec3 ground_irradiance = GetIrradiance(IrradianceTexture, Sampler, Rg, dot(ground_normal, omega_s));
            incident_radiance += transmittance_to_ground *  ground_albedo * (1.0 / PI) * ground_irradiance;
            
            // The radiance finally scattered from direction omega_i towards direction
            // -omega is the product of the incident radiance, the scattering
            // coefficient, and the phase function for directions omega and omega_i
            // (all this summed over all particle types, i.e. Rayleigh and Mie).
            float nu2 = dot(omega, omega_i);
            float h = r - Rg;
            vec3 rayleigh = Bsr * GetDensityAtLayer(h, RLayer) * RayleighPhaseFunction(nu2);
            vec3 mie = Bsm * GetDensityAtLayer(h, MLayer) * MiePhaseFunction(nu2);
            rayleigh_mie += incident_radiance * (rayleigh + mie) * domega_i;
        }
    }
    return rayleigh_mie;
}

vec4 PixelMain(geometry_output In) : SV_TARGET
{
    vec3 FragCoords = vec3(In.Position.xy, In.SliceId + 0.5f);
    float r, mu, mu_s, nu;
    bool IntersectsGround;
    GetRMuMuSNuFromFragCoords(FragCoords, r, mu, mu_s, nu, IntersectsGround);
    vec3 Result = ComputeScatteringDensity(r, mu, mu_s, nu);
    
    return vec4(Result, 1.0f);
}