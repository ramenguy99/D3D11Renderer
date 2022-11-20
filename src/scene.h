#define MAX_MATERIALS_COUNT 256
#define MAX_MESHES_COUNT 256

#define MIN_SHADOW_BIAS 0.001f
#define MAX_SHADOW_BIAS 0.005f

struct mesh
{
    char* Name;
    
    mesh_data* MeshData;
    mesh_gpu* GpuMesh;
    b32 CastsShadows;
    u32 MaterialIndex;
    
    vec3 Position;
    vec3 Rotation; //Rotation along X, Y and Z axis (EulerXYZ)
    vec3 Scale;
    
    mat4 DrawTransform;
    aabb AABB; 
    b32 AABBDirty;
};

// IMPORTANT: The names, components, kind and order in the material struct MUST match
char* MaterialTextureNames[] = {
    "albedo",
    "normal",
    "roughness",
    "metallic",
    "ao",
    "emissive",
};

u32 MaterialTextureComponentsCount[] = {
    4,
    4,
    1,
    1,
    1,
    4,
};

enum material_texture_kind
{
    ALBEDO,
    NORMAL,
    ROUGHNESS,
    METALLIC,
    AO,
    EMISSIVE,
    
    MATERIAL_TEXTURES_COUNT
};

struct material
{
    char* Name;
    union {
        struct{
            texture AlbedoTexture;
            texture NormalTexture;
            texture RoughnessTexture;
            texture MetallicTexture;
            texture AOTexture;
            texture EmissiveTexture;
        };
        
        texture Textures[MATERIAL_TEXTURES_COUNT];
    };
    
    union {
        struct {
            b32 HasAlbedo;
            b32 HasNormal;
            b32 HasRoughness;
            b32 HasMetallic;
            b32 HasAO;
            b32 HasEmissive;
        };
        
        b32 HasTexture[MATERIAL_TEXTURES_COUNT];
    };
    
    //Values used when no texture
    vec3 Albedo;
    f32 Roughness;
    f32 Metallic;    
};

struct point_light
{
    vec3 Position;
    f32 Radius;
    vec3 Color;
    
    shadow_cubemap ShadowCubemap;
    
    bool DebugDrawLightPosition;
    bool DebugDrawFrustum;
    s32 DebugFrustumFace;
};

struct directional_light
{
    vec3 Direction;
    vec3 Color;
    
    shadow_map ShadowMap;
    vec3 ShadowPosition; //Used to define the shadow frustum
    vec2 ShadowFrustumSize;
    f32 ShadowFrustumNear;
    f32 ShadowFrustumFar;
    
    bool DebugDrawFrustum;
};

struct scene
{
    vec3 ViewPosition;
    vec3 CameraForward;
    vec3 PerspectiveHor;
    vec3 PerspectiveVer;
    float CameraNear;
    float CameraFar;
    
    mat4 Projection;
    mat4 View;
    frustum CameraFrustum;
    
    mesh Meshes[MAX_MESHES_COUNT];
    u32 MeshesCount;
    
    material Materials[MAX_MATERIALS_COUNT];
    u32 MaterialsCount;
    
    point_light PointLights[MAX_POINT_LIGHTS_COUNT];
    u32 PointLightsCount;
    
    directional_light DirectionalLights[MAX_DIRECTIONAL_LIGHTS_COUNT];
    u32 DirectionalLightsCount;
    
    light_probe Probe;
    atmosphere Atmosphere;
    
    vec3 SunDirection;
    vec3 SunIlluminanceColor;
    float SunIlluminanceScale;
};