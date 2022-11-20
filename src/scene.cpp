#include "scene.h"

internal mesh*
AddMesh(scene* Scene, char* Name, mesh_data* MeshData,  mesh_gpu* Gpu, u32 MaterialIndex, b32 CastsShadows = true)
{
    Assert(Scene->MeshesCount < MAX_MESHES_COUNT);
    mesh* Mesh = Scene->Meshes + Scene->MeshesCount++;
    Mesh->Name = Name;
    Mesh->MeshData = MeshData;
    Mesh->GpuMesh = Gpu;
    Mesh->MaterialIndex = MaterialIndex;
    Mesh->CastsShadows = CastsShadows;
    Mesh->Scale = vec3(1.0f);
    Mesh->AABBDirty = true;
    
    return Mesh;
}

internal void
AddMaterial(scene* Scene, material* Material)
{
    Assert(Scene->MaterialsCount < MAX_MATERIALS_COUNT);
    material* New = Scene->Materials + Scene->MaterialsCount++;
    *New = *Material;
}

internal directional_light*
AddDirectionalLight(scene* Scene, vec3 Direction, vec3 Color, vec3 ShadowFrustumPosition, vec3 ShadowFrustumSize, shadow_map ShadowMap)
{
    Assert(Scene->DirectionalLightsCount < MAX_DIRECTIONAL_LIGHTS_COUNT);
    directional_light* Light = Scene->DirectionalLights + Scene->DirectionalLightsCount++;
    Light->Direction = Direction;
    Light->Color = Color;
    Light->ShadowMap = ShadowMap;
    Light->ShadowPosition = ShadowFrustumPosition;
    Light->ShadowFrustumSize = vec2(ShadowFrustumSize.x, ShadowFrustumSize.y);
    Light->ShadowFrustumNear = 0.1f;
    Light->ShadowFrustumFar = ShadowFrustumSize.z;
    
    return Light;
}

internal void
AddPointLight(scene* Scene, vec3 Position, f32 Radius, vec3 Color, shadow_cubemap ShadowCubemap)
{
    Assert(Scene->DirectionalLightsCount < MAX_POINT_LIGHTS_COUNT);
    point_light* Light = Scene->PointLights + Scene->PointLightsCount++;
    Light->Color = Color;
    Light->Position = Position;
    Light->Radius = Radius;
    Light->ShadowCubemap = ShadowCubemap;
    Light->DebugDrawLightPosition = true;
}

internal mat4
GetShadowMatrix(directional_light* Light)
{
    vec2 HalfSize = Light->ShadowFrustumSize * 0.5f;
    f32 Near = Light->ShadowFrustumNear;
    f32 Far = Light->ShadowFrustumFar;
    mat4 LightProj = Mat4Orthographic(-HalfSize.x, HalfSize.x, -HalfSize.y, HalfSize.y, Near, Far);
    mat4 LightView = Mat4LookAt(Light->ShadowPosition, Light->ShadowPosition + Light->Direction, vec3(0.0f, 0.0f, 1.0f));
    mat4 Result = LightProj * LightView;    
    return Result;
}


internal texture
LoadTextureFromAssetFile(ID3D11Device* Device, HANDLE File, asset_table Table, char* Name, DXGI_FORMAT Format)
{
    texture Result = {};
    
    asset_table_entry* Entry = FindAsset(Name, Table, ASSET_IMAGE);
    if(Entry)
    {
        void* Data = ZeroAlloc(Entry->Size);
        Win32_ReadAtOffset(File, Data, Entry->Size, Entry->Offset);
        image_data Image = LoadImageAsset(Data, Entry->Size);
        Result = D3D11_CreateTexture(Device, &Image, Format);
    }
    
    return Result;
}

internal material
LoadMaterialFromAssetFile(ID3D11Device* Device, HANDLE File, asset_table Table, char* Name)
{
    material Result = {};
    Result.Name = Name;
    
    char AssetName[512];
    for(u32 i = 0; i < MATERIAL_TEXTURES_COUNT; i++)
    {
        snprintf(AssetName, sizeof(AssetName), "%s_%s", Name, MaterialTextureNames[i]);
        asset_table_entry* Entry = FindAsset(AssetName, Table, ASSET_IMAGE);
        if(Entry)
        {
            void* Data = ZeroAlloc(Entry->Size);
            Win32_ReadAtOffset(File, Data, Entry->Size, Entry->Offset);
            image_data Image = LoadImageAsset(Data, Entry->Size);
            
            DXGI_FORMAT Format;
            if(Image.BytesPerPixel == 1)
            {
                Format = DXGI_FORMAT_R8_UNORM;
            } 
            else 
            {
                Assert(Image.BytesPerPixel == 4);
                Format = (i == ALBEDO || i == EMISSIVE) ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
            }
            
            Result.Textures[i] = D3D11_CreateTexture(Device, &Image, Format); //Only albedo is SRGB
            Result.HasTexture[i] = true;    
            
            // NOTE: Debug tracking
            char* TrackedName = (char*)ZeroAlloc(strlen(AssetName) + 1);
            strcpy(TrackedName, AssetName);
            ivec2 Size = ivec2(Image.Width, Image.Height);
            PushTrackedTexture(Result.Textures[i].ResourceView, Size, TrackedName, TRACKED_TEXTURE_2D);
            
            Free(Data);
        }
        else
        {
            // NOTE: We only alow AO and EMISSIVE to be missing for now
            Assert(i == AO || i == EMISSIVE);
        }
    }
    
    return Result;
}

internal d3d11_cubemap
LoadCubemap(ID3D11Device* Device, HANDLE File,  asset_table_entry* Entry)
{
    void* Data = ZeroAlloc(Entry->Size);
    Win32_ReadAtOffset(File, Data, Entry->Size, Entry->Offset);
    cubemap_data CubemapData = LoadCubemapAsset(Data, Entry->Size);
    
    d3d11_cubemap Result = D3D11_LoadCubemap(Device, &CubemapData);
    Free(Data);
    
    return Result;
}

internal light_probe
LoadLightProbeFromAssetFile(ID3D11Device* Device, HANDLE File, asset_table Table,  char* Name)
{
    light_probe Result = {};
    
    asset_table_entry* BaseEntry = FindAsset(Name, Table, ASSET_CUBEMAP, "skybox");
    asset_table_entry* IrradianceEntry = FindAsset(Name, Table, ASSET_CUBEMAP, "irradiance");
    asset_table_entry* SpecularEntry = FindAsset(Name, Table, ASSET_CUBEMAP, "specular");
    
    Result.Base = LoadCubemap(Device, File, BaseEntry);
    Result.Irradiance = LoadCubemap(Device, File, IrradianceEntry);
    Result.Specular = LoadCubemap(Device, File, SpecularEntry);
    
    return Result;
}
                            
internal material
LoadMaterial(ID3D11Device* Device, char* Name, char* Extension = ".png")
{
    material Result = {};
    
    char Path[512];
    ivec2 Size;
    
    for(u32 i = 0; i < MATERIAL_TEXTURES_COUNT; i++)
    {
        sprintf(Path, "../res/%s/%s_%s%s", Name, Name, MaterialTextureNames[i], Extension);
        image_data Image = LoadImageFromFile(Path, MaterialTextureComponentsCount[i]);
        //If we have an image
        if(Image.Width != 0)
        {
            DXGI_FORMAT Format;
            if(Image.BytesPerPixel == 1)
            {
                Format = DXGI_FORMAT_R8_UNORM;
            } 
            else 
            {
                Assert(Image.BytesPerPixel == 4);
                Format = (i == ALBEDO || i == EMISSIVE) ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
            }
            
            
            Result.Textures[i] = D3D11_CreateTexture(Device, &Image, Format);
            Result.HasTexture[i] = true;    
            
            // NOTE: Debug tracking
            sprintf(Path, "%s - %s", Name, MaterialTextureNames[i]);
            char* TrackedName = (char*)ZeroAlloc(strlen(Path) + 1);
            strcpy(TrackedName, Path);
            Size = ivec2(Image.Width, Image.Height);
            PushTrackedTexture(Result.Textures[i].ResourceView, Size, TrackedName, TRACKED_TEXTURE_2D);
            
            FreeImage(&Image);
        }
        else 
        {
            // NOTE: We only alow AO and EMISSIVE to be missing for now
            Assert(i == AO || i == EMISSIVE);
        }
    }    
    Result.Name = Name;
    return Result;
}

internal mesh_data
LoadMeshFromAssetFile(HANDLE File, asset_table Table, char* Name, char* Tag = "")
{
    asset_table_entry* Entry = FindAsset(Name, Table, ASSET_MESH, Tag);
    
    void* Data = ZeroAlloc(Entry->Size);
    Win32_ReadAtOffset(File, Data, Entry->Size, Entry->Offset);
    
    mesh_data Result = LoadMeshAsset(Data, Entry->Size);
    
    return Result;
}