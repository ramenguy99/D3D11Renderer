#include "defines.hlsl"
#include "pbr_common.hlsl"

SamplerState SampleType : register(s0);
SamplerComparisonState ShadowSampleType : register(s1);

Texture2D AlbedoTexture : register(t0);
Texture2D MetallicTexture : register(t1);
Texture2D RoughnessTexture : register(t2);
Texture2D NormalTexture : register(t3);
Texture2D AOTexture : register(t4);
Texture2D EmissiveTexture : register(t5);
TextureCube<vec4> IrradianceMap : register(t6);
TextureCube<vec4> SpecularMap : register(t7);
Texture2D<vec2> BRDFTexture : register(t8);

Texture2D<float> ShadowMaps[MAX_DIRECTIONAL_LIGHTS_COUNT];
TextureCube<float> ShadowCubemaps[MAX_POINT_LIGHTS_COUNT];

struct point_light
{
    vec3 Position;
    float Radius;
    vec3 Color;
    
    uint __padding;
};

struct dir_light
{
    vec3 Direction;
    uint __padding0;
    
    vec3 Color;
    uint __padding1;
};

cbuffer pixel_constants
{
    point_light PointLight[MAX_POINT_LIGHTS_COUNT];
    dir_light DirectionalLight[MAX_DIRECTIONAL_LIGHTS_COUNT];
        
    uint HasAlbedo;
    uint HasMetallic;
    uint HasRoughness;
    uint HasAO;
    
    uint HasNormal;
    uint HasEmissisive;
    float MetallicConst;
    float RoughnessConst;
    
    vec3 AlbedoConst;
    float AOConst;
    
    vec3 ViewPos;
    uint ExposureEnabled;
    
    float Exposure;
    float MinBias;
    float MaxBias;
}

float DistributionGGX(vec3 N, vec3 H, float Roughness)
{
    float a      = Roughness*Roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float Roughness)
{
    float r = (Roughness + 1.0f);
    float k = (r*r) / 8.0f;
    
    float num   = NdotV;
    float denom = NdotV * (1.0f - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float Roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx2  = GeometrySchlickGGX(NdotV, Roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, Roughness);
	
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float CosTheta, vec3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - CosTheta, 5.0f);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float Roughness)
{
    float invRough = 1.0f - Roughness;
    return F0 + (max(vec3(invRough, invRough, invRough), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

mat3 Mat3Rotate(vec3 Axis, float Radians)
{
    float t = Radians;
    
    float cos_t = cos(t);
    float sin_t = sin(t);
    float inv_cos = 1 - cos_t;
    
    float x = Axis.x;
    float y = Axis.y;
    float z = Axis.z;
    
    mat3 Result = {
        cos_t + x * x* inv_cos,
        y * x * inv_cos + z * sin_t,
        z * x * inv_cos - y * sin_t,
        
        x * y * inv_cos - z * sin_t,
        cos_t + y * y* inv_cos,
        z * y * inv_cos + x * sin_t,
        
        x * z * inv_cos + y * sin_t,
        y * z * inv_cos - x * sin_t,
        cos_t + z * z * inv_cos,
    };
    
    return Result;
}

float PointShadowComputation(vec3 WorldPos, vec3 LightPos, float Radius, float ViewDistance, TextureCube<float> ShadowCubemap)
{
    vec3 LightToWorld = WorldPos - LightPos;
    float Depth = length(LightToWorld);
    
    vec3 LightDir = normalize(LightToWorld);
    float Bias = 0.0002;
    float CompareValue = Depth / Radius - Bias;
    
    vec3 OffsetDirections[5] = {
        vec3(0,   0,  0),
        vec3(-1,  0,  1),
        vec3( 1,  0, -1),
        vec3( 0, -1,  1),
        vec3( 0,  1, -1),
    };
    
    //PCF with multiple samples, those are quite expensive, doing 5 isntead of 1
    //takes about .5 extra milliseconds on our test scene.
    int SampleCount = 5;
    float DiskRadius = 0.05;
    float Shadow = 0.0f;
    for(int i = 0; i < SampleCount; i++)
    {
        vec3 SampleDirection = LightToWorld + OffsetDirections[i] * DiskRadius;
        Shadow += ShadowCubemap.SampleCmpLevelZero(ShadowSampleType, SampleDirection, CompareValue).r;
    }
    
    Shadow = Shadow / SampleCount;
    
    return Shadow;
}

float DirShadowComputation(vec3 LightDir, vec4 ShadowPos, vec3 Normal, Texture2D<float> ShadowMap)
{
    vec3 ProjCoords = ShadowPos.xyz;
    // transform to [0,1] range
    ProjCoords.xy = ProjCoords.xy * 0.5 + 0.5;
    ProjCoords.y = 1.0f - ProjCoords.y;
    
    if(ProjCoords.z > 1.0f)
        return 1.0f;
    
    float Depth = ProjCoords.z;
    float Bias = max(MaxBias * (1.0f - dot(Normal, -LightDir)), MinBias);
    
    float ShadowMapWidth;
    float ShadowMapHeight;
    ShadowMap.GetDimensions(ShadowMapWidth, ShadowMapHeight);
    vec2 TexelSize = vec2(1.0f / ShadowMapWidth, 1.0f / ShadowMapHeight);
    float Shadow = 0.0;
    
    for(int x = -1; x <= 1; x += 1)
    {
        for(int y = -1; y <= 1; y += 1)
        {
            Shadow += ShadowMap.SampleCmpLevelZero(ShadowSampleType, 
                                                   ProjCoords.xy + vec2(x, y) * TexelSize,
                                                   Depth - Bias).r;
        }
    }
    Shadow /= 9.0f;
    
    return Shadow;
}


vec3 LightComputation(vec3 N, vec3 V, vec3 L, float Attenuation,
                      vec3 LightColor, vec3 Albedo, vec3 F0, float Roughness, float Metallic)
{
    vec3 H = normalize(V + L);
    
    vec3 Radiance   = LightColor.rgb * Attenuation;
    
    float NDF = DistributionGGX(N, H, Roughness);
    float G   = GeometrySmith(N, V, L, Roughness);
    vec3 F  = FresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
    
    vec3 Numerator  = NDF * G * F;
    float Denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
    vec3 Specular   = Numerator / max(Denominator, 0.0001f);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - Metallic;
    
    float NdotL = max(dot(N, L), 0.0f);
    vec3 Lo = (kD * Albedo / PI + Specular) * Radiance * NdotL;
    
    return Lo;
}

struct pixel_output
{
    vec4 Color: SV_Target0;
};

pixel_output PixelMain(pixel_input In)
{
    vec3 T = normalize(In.Tangent);
    vec3 N = normalize(In.Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    //This matrix transforms from TBN to world space
    mat3 TBN = transpose(mat3(T, B, N));
    
    vec3 NormalTBN = HasNormal ? NormalTexture.Sample(SampleType, In.TexCoord).xyz * 2.0 - 1.0 : vec3(0.0f, 0.0f, 1.0f);
    N = mul(TBN, NormalTBN);
    
    vec3 V = normalize(ViewPos - In.WorldPos);
    vec3 R = reflect(-V, N);
    R.x = -R.x;
    
    vec3 Albedo = HasAlbedo ? AlbedoTexture.Sample(SampleType, In.TexCoord).rgb : AlbedoConst;
    float Metallic = HasMetallic ? MetallicTexture.Sample(SampleType, In.TexCoord).r : MetallicConst;
    float Roughness = HasRoughness ? RoughnessTexture.Sample(SampleType, In.TexCoord).r : RoughnessConst;
    float AO = HasAO ? AOTexture.Sample(SampleType, In.TexCoord).r : AOConst;
    vec3 Emissive = HasEmissisive ? EmissiveTexture.Sample(SampleType, In.TexCoord).rgb : 0.0;
    
    vec3 F0 = vec3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, Albedo, Metallic);
    
    vec3 Lo = vec3(0.0f, 0.0f, 0.0f);
    vec4 DrawShadow;
    
    for(int i = 0; i < MAX_POINT_LIGHTS_COUNT; ++i)
    {
        point_light Light = PointLight[i];
        
        vec3 L = normalize(Light.Position - In.WorldPos);
        vec3 WorldPosToLight = Light.Position - In.WorldPos;
        float DistanceSquared = dot(WorldPosToLight, WorldPosToLight);
        float Attenuation = 1.0f / DistanceSquared;
        float ViewDistance = length(ViewPos - In.WorldPos);
        
        float Shadow = PointShadowComputation(In.WorldPos, Light.Position, Light.Radius, ViewDistance,  ShadowCubemaps[i]);
        DrawShadow = vec4(vec3(Shadow, Shadow, Shadow), 1.0f);
        
        Lo += LightComputation(N, V, L, Attenuation, Light.Color, 
                               Albedo, F0, Roughness, Metallic) * Shadow;
    }
    
    for(int i = 0; i < MAX_DIRECTIONAL_LIGHTS_COUNT; ++i)
    {
        dir_light Light = DirectionalLight[i];
        vec3 L = -Light.Direction;
        
        float Shadow = DirShadowComputation(Light.Direction, In.ShadowPos[i], 
                                            normalize(In.Normal), ShadowMaps[i]);
        float Attenuation = 1.0f; //No attenuation for directional lights
        Lo += LightComputation(N, V, L, Attenuation, Light.Color, 
                               Albedo, F0, Roughness, Metallic) * Shadow;
    }
    
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, Roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - Metallic;
    vec3 Irradiance = IrradianceMap.Sample(SampleType, N).rgb;
    vec3 Diffuse    = Irradiance * Albedo;
    
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 PrefilteredColor = SpecularMap.SampleLevel(SampleType, R,  Roughness * MAX_REFLECTION_LOD).rgb;
    vec2 BRDFCoords = vec2(clamp(dot(N, V), 0.0, 0.99), Roughness);
    vec2 BRDF  = BRDFTexture.Sample(SampleType, BRDFCoords).rg;
    vec3 Specular = PrefilteredColor * (F * BRDF.x + BRDF.y);
    
    vec3 Ambient = (kD * Diffuse + Specular) * AO;
    
    vec3 Color = Ambient + Lo + Emissive;
    
    pixel_output Out;
    Out.Color = vec4(Color, 1.0f);
    return Out;
}