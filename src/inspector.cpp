#define MAX_TRACKED_TEXTURES 64
#define MAX_TRACKED_SHADERS_COUNT 64

enum tracked_texture_kind
{
    TRACKED_TEXTURE_2D,
    TRACKED_TEXTURE_3D,
    TRACKED_TEXTURE_CUBEMAP,
    TRACKED_SH_MAP,
};

struct tracked_texture
{
    void* Texture;
    ivec2 Size; // TODO: Can we query this from the api so we don't need to update it
    s32 Depth;  // Only used for 3D textures
    char* Name;
    tracked_texture_kind Kind;
    // TODO: Filter type / color channels info
};

enum tracked_shader_kind
{
    TRACKED_SHADER_NONE,
    TRACKED_SHADER_VERTEX,
    TRACKED_SHADER_PIXEL,
    TRACKED_SHADER_GEOMETRY,
    TRACKED_SHADER_DOMAIN,
    TRACKED_SHADER_HULL,
};

struct tracked_shader
{
    tracked_shader_kind Kind;
    wchar_t* Path;
    u64 LastWriteTime;
    b32 IsValid;
    
    void** Ptr; //Pointer to actual shader
    
    char* ErrorMessage;
};

struct inspector_data
{
    bool ShowEditor = true;
    bool ShowDemoWindow = false;
    
    //Render settings
    bool VSyncEnabled = true;
    bool DepthPrepass = true;
    bool ShadowCubemapFrustum = true;
    bool FrustumCulling = true;
    bool FrustumFrustumCulling = true;
    s32 PlaneIndex = 0;
    float AerialPerspectiveScale = 1.0f;
    
    //SH Test
    vec4 L[9] = {
        vec4(0.55f), vec4(0.24f), vec4(0.06f), vec4(0.43f), vec4(0.18f), 
        vec4(0.18f), vec4(-0.23f), vec4(-0.14f), vec4(-0.07f)
    };
    
    //Data
    s32 ObjectsDrawnOnCubemap = 0;
    s32 ObjectsDrawn = 0;
    
    
    //Tracked textures
    tracked_texture TrackedTextures[MAX_TRACKED_TEXTURES];
    u32 TrackedTexturesCount;
    
    tracked_shader TrackedShaders[MAX_TRACKED_SHADERS_COUNT];
    u32 TrackedShadersCount;    
};

global_variable inspector_data InspectorData;


internal void
PushTrackedTexture(void* Texture, ivec2 Size, char* Name, tracked_texture_kind Kind, s32 Depth = 0)
{
    Assert(InspectorData.TrackedTexturesCount < MAX_TRACKED_TEXTURES);
    tracked_texture* Track = InspectorData.TrackedTextures + InspectorData.TrackedTexturesCount++;
    Track->Size = Size;
    Track->Depth = Depth;
    Track->Texture = Texture;
    Track->Name = Name;
    Track->Kind = Kind;
}

internal u64
Win32_FindLastWriteTime(wchar_t* FileName)
{
    u64 LastWriteTime = 0;
    
    WIN32_FILE_ATTRIBUTE_DATA FileData;
    if(GetFileAttributesExW(FileName, GetFileExInfoStandard, (void*)&FileData))
    {
        LastWriteTime |= FileData.ftLastWriteTime.dwLowDateTime;
        LastWriteTime |= (u64)FileData.ftLastWriteTime.dwHighDateTime << 32;
    }
    
    return(LastWriteTime);
}

internal void
PushTrackedShader(wchar_t* Path, void** Ptr, tracked_shader_kind Kind)
{
    Assert(InspectorData.TrackedShadersCount < ArrayCount(InspectorData.TrackedShaders));
    tracked_shader* Track = &InspectorData.TrackedShaders[InspectorData.TrackedShadersCount++];
    
    Track->Path = Path;
    Track->LastWriteTime = Win32_FindLastWriteTime(Path);
    Track->IsValid = true;
    Track->ErrorMessage = 0;
    Track->Ptr = Ptr;
    Track->Kind = Kind;
}

internal ID3D11VertexShader*
D3D11_LoadVertexShader(ID3D11Device* Device, wchar_t* Path, ID3D10Blob** OutBuffer);

internal ID3D11PixelShader*
D3D11_LoadPixelShader(ID3D11Device* Device, wchar_t* Path);

internal void
CheckShadersForUpdate(ID3D11Device* Device)
{
    for(u32 Index = 0; Index < InspectorData.TrackedShadersCount; Index++)
    {
        tracked_shader* Shader = &InspectorData.TrackedShaders[Index];
        u64 LastWriteTime = Win32_FindLastWriteTime(Shader->Path);
        if(LastWriteTime > Shader->LastWriteTime)
        {
            Shader->LastWriteTime = LastWriteTime;
            
            switch(Shader->Kind) {
                case TRACKED_SHADER_VERTEX: {
                    ID3D10Blob* Buffer = 0;
                    *Shader->Ptr = D3D11_LoadVertexShader(Device, Shader->Path, &Buffer);
                    Shader->IsValid = *Shader->Ptr != 0;
                    if(Buffer) Buffer->Release();
                } break;
                
                case TRACKED_SHADER_PIXEL: {
                    *Shader->Ptr = D3D11_LoadPixelShader(Device, Shader->Path);
                    Shader->IsValid = *Shader->Ptr != 0;
                } break;
            };
        }
    }
}