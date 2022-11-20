#include "../defines.hlsl"

//NOTE: This stuff must all be in sync with the d3d11 layer
#define TRANSMITTANCE_TEXTURE_WIDTH 256
#define TRANSMITTANCE_TEXTURE_HEIGHT 64

#define IRRADIANCE_TEXTURE_WIDTH 64
#define IRRADIANCE_TEXTURE_HEIGHT 16

#define SCATTERING_TEXTURE_R_SIZE 32
#define SCATTERING_TEXTURE_MU_SIZE 128
#define SCATTERING_TEXTURE_MU_S_SIZE 32
#define SCATTERING_TEXTURE_NU_SIZE 8

#define SCATTERING_TEXTURE_WIDTH  (SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE)
#define SCATTERING_TEXTURE_HEIGHT (SCATTERING_TEXTURE_MU_SIZE)
#define SCATTERING_TEXTURE_DEPTH  (SCATTERING_TEXTURE_R_SIZE)

//Integrals sample count
#define TRANSMITTANCE_SAMPLE_COUNT      500
#define SINGLE_SCATTERING_SAMPLE_COUNT   50
#define SCATTERING_DENSITY_SAMPLE_COUNT  16
#define INDIRECT_IRRADIANCE_SAMPLE_COUNT 32
#define MULTIPLE_SCATTERING_SAMPLE_COUNT 32


struct density_layer 
{
    float ExpTerm;
    float ExpScale;
    float LinearTerm;
    float ConstantTerm;
};

cbuffer atmosphere_parameters : register(b0)
{
    float Rg;  //Radius of ground
    float Rt;  //Radius of top of atmosphere
    float Hr;  //Rayleigh height scale
    float Hm;  //Mie height scale
    
    vec3 Bsr;             //Rayleigh scattering coefficient
    float __padding0;
    vec3 Ber;             //Rayleigh extintion coefficient
    float __padding1;
    density_layer RLayer; //Rayleigh density layer
    
    vec3 Bsm;             //Mie scattering coefficient
    float __padding2;
    vec3 Bem;             //Mie extintion coefficient
    float MieG;
    density_layer MLayer; //Mie density layer
    
    vec3 Beo;                 //Ozone-absorption extintion coefficient
    float OLayerHeight;       //Height at which we change the ozone density layer
    density_layer OLayers[2]; //Ozone-absorption density layers
    
    vec3 SolarIrradiance;
    float SunAngularRadius;
    
    float MuSMin;
    vec3 GroundAlbedo;
};

cbuffer scattering_order_constants : register(b1)
{
    int ScatteringOrder;
};


float ClampRadius(float r)
{
    return clamp(r, Rg, Rt);
}

float ClampCosine(float v)
{
    return clamp(v, -1, 1);
}
float DistanceToTopAtmosphereBoundary(float r, float mu) 
{
    float discriminant = r * r * (mu * mu - 1.0) + Rt * Rt;
    return max(-r * mu + SafeSqrt(discriminant), 0.0);
}

float DistanceToBottomAtmosphereBoundary(float r, float mu) 
{
    float discriminant = r * r * (mu * mu - 1.0) + Rg * Rg;
    return max(-r * mu - SafeSqrt(discriminant), 0.0);
}

float DistanceToNearestAtmosphereBoundary(float r, float mu, bool IntersectsGround)
{
    if(IntersectsGround) {
        return DistanceToBottomAtmosphereBoundary(r, mu);
    } else {
        return DistanceToTopAtmosphereBoundary(r, mu);
    }
}

//For a texture of size N maps from [0.5 / N, 1 - 0.5 /N] to [0, 1]
float GetUnitRangeFromTextureCoord(float u, int texture_size) 
{
    return (u - 0.5 / texture_size) / (1.0 - 1.0 / texture_size);
}

//Inverse mapping of the above
float GetTextureCoordFromUnitRange(float x, int texture_size) 
{
    return 0.5 / texture_size + x * (1.0 - 1.0 / texture_size);
}

vec2 GetTransmittanceTextureUVFromRMu(float r, float mu)
{
    // Distance to top atmosphere boundary for a horizontal ray at ground level.
    float H = sqrt(Rt * Rt -
                    Rg * Rg);
    // Distance to the horizon.
    float rho =
        SafeSqrt(r * r - Rg * Rg);
    // Distance to the top atmosphere boundary for the ray (r,mu), and its minimum
    // and maximum values over all mu - obtained for (r,1) and (r,mu_horizon).
    float d = DistanceToTopAtmosphereBoundary(r, mu);
    float d_min = Rt - r;
    float d_max = rho + H;
    float x_mu = (d - d_min) / (d_max - d_min);
    float x_r = rho / H;
    return vec2(GetTextureCoordFromUnitRange(x_mu, TRANSMITTANCE_TEXTURE_WIDTH),
                GetTextureCoordFromUnitRange(x_r, TRANSMITTANCE_TEXTURE_HEIGHT));
}

vec3 GetTransmittanceToTopAtmosphere(Texture2D TransmittanceTexture, SamplerState Sampler, float r, float mu)
{
    vec2 UV = GetTransmittanceTextureUVFromRMu(r, mu);
    return TransmittanceTexture.Sample(Sampler, UV).xyz;
}

vec3 GetTransmittanceToSun(Texture2D TransmittanceTexture, SamplerState Sampler, float r, float mu_s)
{
    
    float sin_theta_h = Rg / r;
    float cos_theta_h = -sqrt(max(1.0 - sin_theta_h * sin_theta_h, 0.0));
    return GetTransmittanceToTopAtmosphere(TransmittanceTexture, Sampler, r, mu_s) *
        smoothstep(-sin_theta_h * SunAngularRadius, sin_theta_h * SunAngularRadius, mu_s - cos_theta_h);
}

vec3 GetTransmittance(Texture2D TransmittanceTexture, SamplerState Sampler, float r, float mu,
                      float d, bool IntersectsGround)
{
    float r_d = ClampRadius(sqrt(d * d + 2.0 * r * mu * d + r * r));
    float mu_d = ClampCosine((r * mu + d) / r_d);
    
    vec3 t;
    if(IntersectsGround) {
        t = GetTransmittanceToTopAtmosphere(TransmittanceTexture, Sampler, r_d, -mu_d) /
            GetTransmittanceToTopAtmosphere(TransmittanceTexture, Sampler, r, -mu);
    } else {
        t = GetTransmittanceToTopAtmosphere(TransmittanceTexture, Sampler, r, mu) /
            GetTransmittanceToTopAtmosphere(TransmittanceTexture, Sampler, r_d, mu_d);
    }
    return min(t, vec3(1.0, 1.0, 1.0));
}

float GetDensityAtLayer(float Altitude, density_layer Layer)
{
    float Density = Layer.ExpTerm * exp(Layer.ExpScale * Altitude) + Layer.LinearTerm * Altitude + Layer.ConstantTerm;
    return clamp(Density, 0.0, 1.0);
}

float GetDensity(float Altitude, density_layer Layer0, density_layer Layer1, float LayerHeight)
{
    if(Altitude < LayerHeight)
        return GetDensityAtLayer(Altitude, Layer0);
    else
        return GetDensityAtLayer(Altitude, Layer1);
}

void GetRMuMuSNuFromScatteringTextureUVWZ(vec4 UVWZ, out float r, out float mu, out float mu_s, out float nu, out bool RayRMuIntersectsGround)
{
    // Distance to top atmosphere boundary for a horizontal ray at ground level.
    float H = sqrt(Rt * Rt - Rg * Rg);
    // Distance to the horizon.
    float rho = H * GetUnitRangeFromTextureCoord(UVWZ.w, SCATTERING_TEXTURE_R_SIZE);
    r = sqrt(rho * rho + Rg * Rg);
    
    if (UVWZ.z < 0.5) {
        // Distance to the ground for the ray (r,mu), and its minimum and maximum
        // values over all mu - obtained for (r,-1) and (r,mu_horizon) - from which
        // we can recover mu:
        float d_min = r - Rg;
        float d_max = rho;
        float d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord(1.0 - 2.0 * UVWZ.z, SCATTERING_TEXTURE_MU_SIZE / 2);
        mu = d == 0.0 ? float(-1.0) : ClampCosine(-(rho * rho + d * d) / (2.0 * r * d));
        RayRMuIntersectsGround = true;
    } else {
        // Distance to the top atmosphere boundary for the ray (r,mu), and its
        // minimum and maximum values over all mu - obtained for (r,1) and
        // (r,mu_horizon) - from which we can recover mu:
        float d_min = Rt - r;
        float d_max = rho + H;
        float d = d_min + (d_max - d_min) * GetUnitRangeFromTextureCoord( 2.0 * UVWZ.z - 1.0, SCATTERING_TEXTURE_MU_SIZE / 2);
        mu = d == 0.0 ? float(1.0) : ClampCosine((H * H - rho * rho - d * d) / (2.0 * r * d));
        RayRMuIntersectsGround = false;
    }
    
    float x_mu_s = GetUnitRangeFromTextureCoord(UVWZ.y, SCATTERING_TEXTURE_MU_S_SIZE);
    float d_min = Rt - Rg;
    float d_max = H;
    float A = -2.0 * MuSMin * Rg / (d_max - d_min);
    float a = (A - x_mu_s * A) / (1.0 + x_mu_s * A);
    float d = d_min + min(a, A) * (d_max - d_min);
    mu_s = d == 0.0 ? float(1.0) : ClampCosine((H * H - d * d) / (2.0 * Rg * d));
    
    nu = ClampCosine(UVWZ.x * 2.0 - 1.0);    
}

vec4 GetUVWZFromScatteringTextureFragCoord(vec3 FragCoord)
{
    vec4 TextureSize = vec4(SCATTERING_TEXTURE_NU_SIZE - 1,
                            SCATTERING_TEXTURE_MU_S_SIZE,
                            SCATTERING_TEXTURE_MU_SIZE,
                            SCATTERING_TEXTURE_R_SIZE);
    //Map X for a 4D texture
    float FragCoordNu = floor(FragCoord.x / SCATTERING_TEXTURE_MU_S_SIZE);
    float FragCoordMuS = fmod(FragCoord.x, SCATTERING_TEXTURE_MU_S_SIZE);
    
    vec4 UVWZ = vec4(FragCoordNu, FragCoordMuS, FragCoord.yz) / TextureSize;
    return UVWZ;
}

void GetRMuMuSNuFromFragCoords(vec3 FragCoords, out float r, out float mu, out float mu_s, out float nu, out bool IntersectsGround)
{
    vec4 UVWZ = GetUVWZFromScatteringTextureFragCoord(FragCoords);
    GetRMuMuSNuFromScatteringTextureUVWZ(UVWZ, r, mu, mu_s, nu, IntersectsGround);
    // Clamp nu to its valid range of values, given mu and mu_s.
    nu = clamp(nu, mu * mu_s - sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)), 
                   mu * mu_s + sqrt((1.0 - mu * mu) * (1.0 - mu_s * mu_s)));
}

vec4 GetScatteringTextureUvwzFromRMuMuSNu(float r, float mu, float mu_s, float nu,
                                          bool ray_r_mu_intersects_ground) 
{
    // Distance to top atmosphere boundary for a horizontal ray at ground level.
    float H = sqrt(Rt * Rt -  Rg * Rg);
    // Distance to the horizon.
    float rho = SafeSqrt(r * r - Rg * Rg);
    float u_r = GetTextureCoordFromUnitRange(rho / H, SCATTERING_TEXTURE_R_SIZE);
    
    // Discriminant of the quadratic equation for the intersections of the ray
    // (r,mu) with the ground (see RayIntersectsGround).
    float r_mu = r * mu;
    float discriminant = r_mu * r_mu - r * r + Rg * Rg;
    float u_mu;
    if (ray_r_mu_intersects_ground) {
        // Distance to the ground for the ray (r,mu), and its minimum and maximum
        // values over all mu - obtained for (r,-1) and (r,mu_horizon).
        float d = -r_mu - SafeSqrt(discriminant);
        float d_min = r - Rg;
        float d_max = rho;
        u_mu = 0.5 - 0.5 * GetTextureCoordFromUnitRange(d_max == d_min ? 0.0 :  (d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
    } else {
        // Distance to the top atmosphere boundary for the ray (r,mu), and its
        // minimum and maximum values over all mu - obtained for (r,1) and
        // (r,mu_horizon).
        float d = -r_mu + SafeSqrt(discriminant + H * H);
        float d_min = Rt - r;
        float d_max = rho + H;
        u_mu = 0.5 + 0.5 * GetTextureCoordFromUnitRange((d - d_min) / (d_max - d_min), SCATTERING_TEXTURE_MU_SIZE / 2);
    }
    
    float d = DistanceToTopAtmosphereBoundary(Rg, mu_s);
    float d_min = Rt - Rg;
    float d_max = H;
    float a = (d - d_min) / (d_max - d_min);
    float A =
        -2.0 * MuSMin * Rg / (d_max - d_min);
    float u_mu_s = GetTextureCoordFromUnitRange(max(1.0 - a / A, 0.0) / (1.0 + a), SCATTERING_TEXTURE_MU_S_SIZE);
    
    float u_nu = (nu + 1.0) / 2.0;
    return vec4(u_nu, u_mu_s, u_mu, u_r);
}

bool RayIntersectsGround(float r, float mu)
{
    return mu < 0.0 && r * r * (mu * mu - 1.0) + Rg * Rg >= 0.0;
}

float RayleighPhaseFunction(float nu) {
    float k = 3.0 / (16.0 * PI);
    return k * (1.0 + nu * nu);
}

float MiePhaseFunction(float nu) {
    float g = MieG;
    float k = 3.0 / (8.0 * PI) * (1.0 - g * g) / (2.0 + g * g);
    return k * (1.0 + nu * nu) / pow(1.0 + g * g - 2.0 * g * nu, 1.5);
}

vec3 GetScattering(Texture3D ScatteringTexture, SamplerState Sampler,  float r, float mu, float mu_s, 
                   float nu, bool ray_r_mu_intersects_ground)
{
    vec4 uvwz = GetScatteringTextureUvwzFromRMuMuSNu(r, mu, mu_s, nu, ray_r_mu_intersects_ground);
    float tex_coord_x = uvwz.x * float(SCATTERING_TEXTURE_NU_SIZE - 1);
    float tex_x = floor(tex_coord_x);
    float t = tex_coord_x - tex_x;
    vec3 uvw0 = vec3((tex_x + uvwz.y) / float(SCATTERING_TEXTURE_NU_SIZE),
                     uvwz.z, uvwz.w);
    vec3 uvw1 = vec3((tex_x + 1.0 + uvwz.y) / float(SCATTERING_TEXTURE_NU_SIZE),
                     uvwz.z, uvwz.w);
    return ScatteringTexture.Sample(Sampler, uvw0).rgb * (1.0 - t) +
           ScatteringTexture.Sample(Sampler, uvw1).rgb * t;
}

vec3 GetScattering(Texture3D RScatteringTexture, Texture3D MScatteringTexture, Texture3D MultipleScatteringTexture, SamplerState Sampler,
                   float r, float mu, float mu_s, float nu, bool ray_r_mu_intersects_ground, uint order) {
    if (order == 1) {
        vec3 rayleigh = GetScattering(RScatteringTexture, Sampler, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
        vec3 mie = GetScattering(MScatteringTexture, Sampler, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
        return rayleigh * RayleighPhaseFunction(nu) + mie * MiePhaseFunction(nu);
    } else {
        return GetScattering(MultipleScatteringTexture, Sampler, r, mu, mu_s, nu, ray_r_mu_intersects_ground);
    }
}

vec2 GetIrradianceTextureUvFromRMuS(float r, float mu_s)
{
    float x_r = (r - Rg) / (Rt - Rg);
    float x_mu_s = mu_s * 0.5 + 0.5;
    return vec2(GetTextureCoordFromUnitRange(x_mu_s, IRRADIANCE_TEXTURE_WIDTH),
                GetTextureCoordFromUnitRange(x_r, IRRADIANCE_TEXTURE_HEIGHT));
}

vec3 GetIrradiance(Texture2D IrradianceTexture, SamplerState Sampler, float r, float mu_s) 
{
    vec2 uv = GetIrradianceTextureUvFromRMuS(r, mu_s);
    return IrradianceTexture.Sample(Sampler, uv).rgb;
}    

void GetRMuSFromIrradianceTextureUV(vec2 UV, out float r, out float mu_s)
{
    float x_mu_s = GetUnitRangeFromTextureCoord(UV.x, IRRADIANCE_TEXTURE_WIDTH);
    float x_r    = GetUnitRangeFromTextureCoord(UV.y, IRRADIANCE_TEXTURE_HEIGHT);
    r = Rg + x_r * (Rt - Rg);
    mu_s = ClampCosine(2.0 * x_mu_s - 1.0);
}


// https://gist.github.com/wwwtyro/beecc31d65d1004f5a9d
// - r0: ray origin
// - rd: normalized ray direction
// - s0: sphere center
// - sR: sphere radius
// - Returns distance from r0 to first intersecion with sphere,
//   or -1.0 if no intersection.
float RaySphereIntersect(vec3 r0, vec3 rd, vec3 s0, float sR)
{
	float a = dot(rd, rd);
	vec3 s0_r0 = r0 - s0;
	float b = 2.0 * dot(rd, s0_r0);
	float c = dot(s0_r0, s0_r0) - (sR * sR);
	if (b*b - 4.0*a*c < 0.0) 
	{
		return -1.0;
	}
	return (-b - sqrt((b*b) - 4.0*a*c)) / (2.0*a);
}
