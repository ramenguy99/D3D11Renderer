#include "atmosphere_common.hlsl"

Texture2D TransmittanceTexture : register(t0);
Texture2D IrradianceTexture    : register(t1);
Texture3D ScatteringTexture    : register(t2);

Texture2D DepthBuffer          : register(t3);

SamplerState Sampler;

cbuffer pixel_constants : register(b2)
{
    vec3 SunDirection; //In world space
    vec3 SunIlluminance;
    float AerialPerspectiveScale;
    
    //Used to reconstruct world direction and position from pixel coordinates
    vec3 CameraPos;
    vec3 CameraForward;
    vec3 PerspectiveHor;
    float PerspectiveA;
    vec3 PerspectiveVer;
    float PerspectiveB;
};

struct pixel_input
{
    vec4 Position : SV_POSITION;
    vec2 TexCoord : TEXCOORD0;
};

vec3 GetExtrapolatedSingleMieScattering(vec4 scattering) {
    if (scattering.r == 0.0) {
        return vec3(0.0, 0.0, 0.0);
    }
    return scattering.rgb * scattering.a / scattering.r * (Bsr.r / Bsm.r) * (Bsm / Bsr);
}

vec3 GetCombinedScattering(float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground,
                           out vec3 single_mie_scattering)
{
    vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(r, mu, mu_s, nu, ray_r_mu_intersects_ground);
    float tex_coord_x = uvwz.x * float(SCATTERING_TEXTURE_NU_SIZE - 1);
    float tex_x = floor(tex_coord_x);
    float t = tex_coord_x - tex_x;
    
    vec3 uvw0 = vec3((tex_x + uvwz.y) / float(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w);
    vec3 uvw1 = vec3((tex_x + 1.0 + uvwz.y) / float(SCATTERING_TEXTURE_NU_SIZE), uvwz.z, uvwz.w);
    vec4 combined_scattering = ScatteringTexture.Sample(Sampler, uvw0) * (1.0 - t) + ScatteringTexture.Sample(Sampler, uvw1) * t;
    vec3 scattering = combined_scattering.rgb;
    single_mie_scattering = GetExtrapolatedSingleMieScattering(combined_scattering);
    
    return scattering;
}

vec3 GetSkyRadiance(vec3 camera, vec3 view_ray, float shadow_length, out vec3 transmittance)
{
    // Compute the distance to the top atmosphere boundary along the view ray,
    // assuming the viewer is in space (or NaN if the view ray does not intersect
    // the atmosphere).
    float r = length(camera);
    float rmu = dot(camera, view_ray);
    float distance_to_top_atmosphere_boundary = -rmu - sqrt(rmu * rmu - r * r + Rt * Rt);
    
    // If the viewer is in space and the view ray intersects the atmosphere, move
    // the viewer to the top atmosphere boundary (along the view ray):
    if (distance_to_top_atmosphere_boundary > 0.0) {
        camera = camera + view_ray * distance_to_top_atmosphere_boundary;
        r = Rt;
        rmu += distance_to_top_atmosphere_boundary;
    } else if (r > Rt) {
        // If the view ray does not intersect the atmosphere, simply return 0.
        transmittance = vec3(1.0, 1.0, 1.0);
        return vec3(0.0, 0.0, 0.0);
    }
    
    // Compute the r, mu, mu_s and nu parameters needed for the texture lookups.
    float mu = rmu / r;
    vec3 sun_direction = SunDirection;
    float mu_s = dot(camera, sun_direction) / r;
    float nu = dot(view_ray, sun_direction);
    bool ray_r_mu_intersects_ground = RayIntersectsGround(r, mu);
    
    transmittance = ray_r_mu_intersects_ground ? vec3(0.0, 0.0, 0.0) : GetTransmittanceToTopAtmosphere(TransmittanceTexture, Sampler, r, mu);
    vec3 single_mie_scattering;
    vec3 scattering;
    if (shadow_length == 0.0) {
        scattering = GetCombinedScattering(r, mu, mu_s, nu, ray_r_mu_intersects_ground,
                                           single_mie_scattering);
    } else {
        // Case of light shafts (shadow_length is the total length noted l in our
        // paper): we omit the scattering between the camera and the point at
        // distance l, by implementing Eq. (18) of the paper (shadow_transmittance
        // is the T(x,x_s) term, scattering is the S|x_s=x+lv term).
        float d = shadow_length;
        float r_p = ClampRadius(sqrt(d * d + 2.0 * r * mu * d + r * r));
        float mu_p = (r * mu + d) / r_p;
        float mu_s_p = (r * mu_s + d * nu) / r_p;
        
        scattering = GetCombinedScattering(r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
                                           single_mie_scattering);
        vec3 shadow_transmittance = GetTransmittance(TransmittanceTexture, Sampler,
                                                     r, mu, shadow_length, ray_r_mu_intersects_ground);
        scattering = scattering * shadow_transmittance;
        single_mie_scattering = single_mie_scattering * shadow_transmittance;
    }
    
    return scattering * RayleighPhaseFunction(nu) + single_mie_scattering * MiePhaseFunction(nu);
}

vec3 GetSkyRadianceToPoint(vec3 camera, vec3 thePoint, float shadow_length, out vec3 transmittance)
{
    vec3 sun_direction = SunDirection;
    
    // Compute the distance to the top atmosphere boundary along the view ray,
    // assuming the viewer is in space (or NaN if the view ray does not intersect
    // the atmosphere).
    vec3 view_ray = normalize(thePoint - camera);
    float r = length(camera);
    float rmu = dot(camera, view_ray);
    float distance_to_top_atmosphere_boundary = -rmu - sqrt(rmu * rmu - r * r + Rt * Rt);
    // If the viewer is in space and the view ray intersects the atmosphere, move
    // the viewer to the top atmosphere boundary (along the view ray):
    if (distance_to_top_atmosphere_boundary > 0.0) {
        camera = camera + view_ray * distance_to_top_atmosphere_boundary;
        r = Rt;
        rmu += distance_to_top_atmosphere_boundary;
    }
    
    // Compute the r, mu, mu_s and nu parameters for the first texture lookup.
    float mu = rmu / r;
    float mu_s = dot(camera, sun_direction) / r;
    float nu = dot(view_ray, sun_direction);
    float d = length(thePoint - camera);
    bool ray_r_mu_intersects_ground = RayIntersectsGround(r, mu);
    
    transmittance = GetTransmittance(TransmittanceTexture, Sampler,
                                     r, mu, d, ray_r_mu_intersects_ground);
    
    vec3 single_mie_scattering;
    vec3 scattering = GetCombinedScattering(r, mu, mu_s, nu, ray_r_mu_intersects_ground,
                                                          single_mie_scattering);
    
    // Compute the r, mu, mu_s and nu parameters for the second texture lookup.
    // If shadow_length is not 0 (case of light shafts), we want to ignore the
    // scattering along the last shadow_length meters of the view ray, which we
    // do by subtracting shadow_length from d (this way scattering_p is equal to
    // the S|x_s=x_0-lv term in Eq. (17) of our paper).
    d = max(d - shadow_length, 0.0);
    float r_p = ClampRadius(sqrt(d * d + 2.0 * r * mu * d + r * r));
    float mu_p = (r * mu + d) / r_p;
    float mu_s_p = (r * mu_s + d * nu) / r_p;
    
    vec3 single_mie_scattering_p;
    vec3 scattering_p = GetCombinedScattering(r_p, mu_p, mu_s_p, nu, ray_r_mu_intersects_ground,
                                                            single_mie_scattering_p);
    
    // Combine the lookup results to get the scattering between camera and point.
    vec3 shadow_transmittance = transmittance;
    if (shadow_length > 0.0) {
        // This is the T(x,x_s) term in Eq. (17) of our paper, for light shafts.
        shadow_transmittance = GetTransmittance(TransmittanceTexture, Sampler,
                                                r, mu, d, ray_r_mu_intersects_ground);
    }
    scattering = scattering - shadow_transmittance * scattering_p;
    single_mie_scattering = single_mie_scattering - shadow_transmittance * single_mie_scattering_p;
    single_mie_scattering = GetExtrapolatedSingleMieScattering(vec4(scattering, single_mie_scattering.r));
    
    // Hack to avoid rendering artifacts when the sun is below the horizon.
    single_mie_scattering = single_mie_scattering * smoothstep(float(0.0), float(0.25), mu_s);
    
    return scattering * RayleighPhaseFunction(nu) + single_mie_scattering *  MiePhaseFunction(nu);
}

vec3 GetWorldDirectionFromNormalizedCoords(vec2 Coords)
{
    vec2 Offset = Coords * 2.0f - 1.0f;
    vec3 Result = CameraForward + Offset.x * PerspectiveHor - Offset.y * PerspectiveVer;
    return normalize(Result);
}

//From Depth buffer value to Z distance from the camera (aka Z in view space)
float LinearizeDepth(float Depth)
{    
    float Result = PerspectiveB / (Depth - PerspectiveA);
    return Result;
}

vec3 GetWorldPositionFromViewRayAndDepth(vec3 V, float Depth)
{
    float LinearDepth = LinearizeDepth(Depth);
    vec3 Result = CameraPos + V * (LinearDepth / dot(V, CameraForward));
    return Result;
}

vec3 WorldToRPos(vec3 Pos)
{
    return Pos * 0.001f + vec3(0, 0, Rg);
}

vec4 PixelMain(pixel_input In) : SV_TARGET
{
    vec2 FragCoords = In.Position.xy;
    float FragDepth = DepthBuffer[FragCoords].r;
    
    vec3 WorldDir = GetWorldDirectionFromNormalizedCoords(In.TexCoord);
    
    
    //Find distance to ground or distance to the first object
    vec3 CameraR = WorldToRPos(CameraPos);
    float t = RaySphereIntersect(CameraR, WorldDir, vec3(0, 0, 0), Rg);
    
    if(FragDepth < 1.0)
    {
        vec3 WorldPos = GetWorldPositionFromViewRayAndDepth(WorldDir, FragDepth);
        vec3 WorldR = WorldToRPos(WorldPos);
        float Distance = length(WorldR - CameraR);
        if(t < 0.0 || Distance < t)
        {
            t = Distance;
        }
    }
    
    vec3 Transmittance = 0.0f;
    vec3 SunTransmittance = 0.0f;
	vec3 SunIlluminanceToGround = 0.0f;
	vec3 SunIlluminanceToSky = 0.0f;
    float ShadowLength = 0.0;
    
    vec3 Color = vec3(1, 0, 0);
    //If we hit the ground
    if(t > 0.0)
    {
        vec3 WorldPosR = CameraR + t * WorldDir;
        SunIlluminanceToGround = GetSkyRadianceToPoint(CameraR, WorldPosR, ShadowLength, Transmittance);
        SunIlluminanceToGround *= AerialPerspectiveScale;
    }
    else
    {            
        SunIlluminanceToSky = GetSkyRadiance(CameraR, WorldDir, ShadowLength, Transmittance);
        SunTransmittance = Transmittance;
    }
    
    vec3 SunLuminance = 0.0;
    if (dot(WorldDir, SunDirection) > cos(0.5*0.505*3.14159 / 180.0))
    {
        SunLuminance = 100.0;
    }
    vec3 Luminance = SunIlluminance * (SunIlluminanceToSky + SunIlluminanceToGround) + SunLuminance * SunTransmittance;
    vec4 Result = vec4(Luminance, 1.0f - dot(Transmittance, vec3(0.33, 0.33, 0.34)));
    
    return Result;
}