#define NEAR_FAR_PLANE_INVERSION false
#define MESH_WINDING_COUNTER_CLOCKWISE false

#define BRDF_SIZE 1024
#define HDR_CUBEMAP_SIZE 1024
#define IRRADIANCE_CUBEMAP_SIZE 32
#define SPECULAR_CUBEMAP_SIZE 256

#if 1
#define ATMOSPHERE_LUT_FORMAT DXGI_FORMAT_R32G32B32A32_FLOAT
#define ATMOSPHERE_LUT_FORMAT_SIZE 16
#else
#define ATMOSPHERE_LUT_FORMAT DXGI_FORMAT_R16G16B16A16_FLOAT
#define ATMOSPHERE_LUT_FORMAT_SIZE 8
#endif

struct d3d11_mesh 
{
    union {
        ID3D11Buffer* VertexBuffers[6];
        struct {
            ID3D11Buffer* PositionsBuffer;
            ID3D11Buffer* NormalsBuffer;
            ID3D11Buffer* TangentsBuffer;
            ID3D11Buffer* UVsBuffer;
            ID3D11Buffer* WeightsBuffer;
            ID3D11Buffer* JointsBuffer;
        };
    };
    u32 VerticesCount;
    
    ID3D11Buffer* IndexBuffer;
    u32 IndicesCount;
    
    mesh_flag Flags;
};

struct d3d11_monitor_info
{
    s32 Width;
    s32 Height;
    f32 RefreshRate;
};

struct d3d11_cubemap_vertex_constants
{
    mat4 Projection;
    mat4 View;
    mat4 Model;
};

struct d3d11_cubemap_pixel_constants
{
    u32 MipLevel;
};

//States common to all render pipelines
struct d3d11_common
{
    ID3D11DepthStencilState* LessDepthStencilState;
    ID3D11DepthStencilState* GreaterDepthStencilState;
    ID3D11DepthStencilState* EqualDepthStencilState;
    
    ID3D11RasterizerState* SolidRasterState;
    ID3D11RasterizerState* WireRasterState;
    ID3D11RasterizerState* SolidRasterStateNoCull;
    
    ID3D11SamplerState* LinearSamplerState;
    ID3D11SamplerState* AnisotropicSamplerState;
    ID3D11RasterizerState* ShadowRasterState;
    ID3D11SamplerState* PointSamplerState;
    ID3D11SamplerState* ShadowSamplerState;
    
    //Basic {vec3 P; vec2 UV} input layout, usually used to render intermediate targets
    ID3D11VertexShader* FrameVertexShader;
    ID3D11InputLayout* FrameLayout; 
    ID3D11Buffer* FrameVertexBuffer;
    
    //Basic sphere of {vec3 P} input layout, indexed and TRIANGLE_STRIP
    ID3D11Buffer* SphereVertexBuffer;
    ID3D11Buffer* SphereIndexBuffer;
    u32 SphereIndicesCount;
    
    
    //Same as above for cubemaps or spheres
    ID3D11Buffer* CubemapVertexBuffer;
    ID3D11VertexShader* CubemapVertexShader;
    ID3D11PixelShader* CubemapPixelShader;
    ID3D11InputLayout* CubemapLayout; 
    ID3D11Buffer* CubemapVertexConstantsBuffer;
    ID3D11Buffer* CubemapPixelConstantsBuffer;
    
    //Shader and constant buffers for debug draw of a 3D texture slice
    ID3D11PixelShader* Texture3DSlicePixelShader;
    ID3D11Buffer* Texture3DSliceConstantsBuffer;
};

struct d3d11_cubemap
{
    ID3D11ShaderResourceView* ResourceView;
    ID3D11Texture2D* Texture;
};

struct d3d11_cubemap_target
{
    ID3D11ShaderResourceView* ResourceView;
    ID3D11Texture2D* Texture;
    ID3D11RenderTargetView* RenderTargets[6];
};

struct d3d11_cubemap_target_mips
{
    ID3D11ShaderResourceView* ResourceView;
    ID3D11Texture2D* Texture;
    //6 faces, 5 mipmaps each
    ID3D11RenderTargetView* RenderTargets[6][5];
};

struct d3d11_depth_buffer
{
    ID3D11DepthStencilView* DepthView;
    ID3D11Texture2D* Texture;
};

struct d3d11_render_target 
{
    ID3D11ShaderResourceView* ResourceView;
    ID3D11Texture2D* Texture;
    ID3D11RenderTargetView* RenderTarget;
    s32 Width;
    s32 Height;
};

struct d3d11_render_target_3d
{
    ID3D11ShaderResourceView* ResourceView;
    ID3D11Texture3D* Texture;
    ID3D11RenderTargetView* RenderTarget;
    s32 Width;
    s32 Height;
    s32 Depth;
};

struct d3d11_shadow_map
{
    ID3D11ShaderResourceView* ResourceView;
    ID3D11Texture2D* Texture;
    ID3D11DepthStencilView* DepthView;
    s32 Width, Height;
};

struct d3d11_shadow_cubemap
{
    ID3D11ShaderResourceView* ResourceView;
    ID3D11Texture2D* Texture;
    ID3D11DepthStencilView* DepthViews[6];
    s32 Size;
};

struct d3d11_texture
{
    ID3D11ShaderResourceView* ResourceView;
    ID3D11Texture2D* Texture;
};


struct d3d11_texture_3d
{
    ID3D11ShaderResourceView* ResourceView;
    ID3D11Texture3D* Texture;
};

struct d3d11_pbr_vertex_constants
{
    mat4 Projection;
    mat4 View;
    mat4 Model;
    mat4 NormalMatrix;
    mat4 ShadowMatrix[MAX_DIRECTIONAL_LIGHTS_COUNT];
};

struct d3d11_pbr_pixel_constants
{
    struct {
        vec3 Position;
        f32 Radius;
        vec3 Color;
        u32 __padding;
    } PointLight[MAX_POINT_LIGHTS_COUNT];
    
    struct {
        vec3 Direction;
        u32 __padding0;
        vec3 Color;
        u32 __padding1;
    } DirectionalLight[MAX_DIRECTIONAL_LIGHTS_COUNT];
    
    b32 HasAlbedo;
    b32 HasMetallic;
    b32 HasRoughness;
    b32 HasAO;
    
    b32 HasNormal;    
    b32 HasEmissive;
    float MetallicConst;
    float RoughnessConst;
    
    vec3 AlbedoConst;
    float AOConst;
    
    vec3 ViewPos;
    b32 ExposureEnabled;
    
    float Exposure;
    float MinBias;
    float MaxBias;
    float __Padding;
};

struct d3d11_pbr_pipeline
{
    ID3D11InputLayout* Layout;
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader*  PixelShader;
    
    ID3D11Buffer* PixelConstantsBuffer;
    ID3D11Buffer* VertexConstantsBuffer;
    
    d3d11_texture BRDFTexture;
    
    d3d11_render_target IntermediateTarget;
    
    ID3D11PixelShader*  PostprocessShader;
};

struct d3d11_light_probe
{
    d3d11_cubemap Base;
    d3d11_cubemap Irradiance;
    d3d11_cubemap Specular;
};

struct d3d11_precompute_vertex_constants
{
    mat4 Projection;
    mat4 View;
};

struct d3d11_precompute_pixel_constants
{
    f32 Roughness;
};

struct d3d11_precompute_pipeline
{    
    ID3D11InputLayout* Layout;
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* EquirectPixelShader;
    ID3D11PixelShader* IrradiancePixelShader;
    ID3D11PixelShader* SpecularPixelShader;
    
    ID3D11Buffer* VertexConstantsBuffer;
    ID3D11Buffer* PixelConstantsBuffer;
};

struct d3d11_shadow_vertex_constants
{
    mat4 Shadow;
    mat4 Model;
};

struct d3d11_shadow_pixel_constants
{
    vec3 LightPosition;
    float FarPlane;
};

struct d3d11_shadow_pipeline
{
    ID3D11InputLayout* Layout;
    ID3D11RasterizerState* RasterState;
    
    ID3D11VertexShader* VertexShader;
    
    ID3D11VertexShader* CubemapVertexShader;
    ID3D11PixelShader* CubemapPixelShader;
    
    ID3D11Buffer* VertexConstantsBuffer;
    ID3D11Buffer* PixelConstantsBuffer;
};

struct color_vertex
{
    vec3 Position;
    vec4 Color;
};

struct d3d11_atmosphere
{
    d3d11_render_target Transmittance;    
    d3d11_render_target Irradiance;
    d3d11_render_target_3d Scattering;
    ID3D11Buffer* ParametersBuffer;
};

struct d3d11_atmosphere_render_constants
{
    vec3 SunDirection;
    float __padding2;
    
    vec3 SunIlluminance;
    float AerialPerspectiveScale;
    
    vec3 CameraPos;
    float __padding0;
    
    vec3 CameraForward;
    float __padding4;
    
    vec3 PerspectiveHor;
    float PerspectiveA;
    
    vec3 PerspectiveVer;
    float PerspectiveB;
};

struct d3d11_atmosphere_pipeline
{
    ID3D11VertexShader* LutVertexShader;
    ID3D11GeometryShader* LutGeometryShader;
    
    ID3D11PixelShader* TransmittanceShader;
    ID3D11PixelShader* DirectIrradianceShader;
    ID3D11PixelShader* SingleScatterShader;
    ID3D11PixelShader* ScatteringDensityShader;
    ID3D11PixelShader* IndirectIrradianceShader;
    ID3D11PixelShader* MultipleScatteringShader;
    
    ID3D11PixelShader* RenderAtmosphereShader;
    
    ID3D11Buffer* ScatteringOrderBuffer;
    ID3D11Buffer* RenderAtmosphereBuffer;
    
    ID3D11BlendState* Blend0Nop1Add;
    ID3D11BlendState* RenderBlend;
    
    //Temp LUTs to generate the atmosphere LUTs
    d3d11_render_target DeltaIrradiance;
    d3d11_render_target_3d DeltaScatteringR;
    d3d11_render_target_3d DeltaScatteringM;
    d3d11_render_target_3d DeltaScatteringDensity;
};

struct d3d11_sh_pipeline
{
    ID3D11VertexShader* TestVertexShader;
    ID3D11PixelShader* TestPixelShader;
    ID3D11VertexShader* LightVertexShader;
    ID3D11PixelShader* LightPixelShader;
    
    ID3D11Buffer* PixelConstantsBuffer;
};

struct d3d11_debug_pipeline
{
    ID3D11InputLayout* ColorLayout;
    ID3D11VertexShader* ColorVertex;
    ID3D11PixelShader* ColorPixel;
    
    ID3D11Buffer* LineVertices;
    ID3D11Buffer* TriangleVertices;
    ID3D11Buffer* VertexConstantsBuffer;
    
    u32 LineIndex;
    u32 TriangleIndex;
};

enum d3d11_profiler_area
{
    D3D11_PROFILE_SHADOWMAP,
    D3D11_PROFILE_SHADOWCUBEMAP,
    D3D11_PROFILE_DEPTH_PREPASS,
    D3D11_PROFILE_MESHES,
    D3D11_PROFILE_DEBUG_DRAW,
    D3D11_PROFILE_ATMOSPHERE,
    D3D11_PROFILE_POSTPROCESS,
    
    D3D11_PROFILE_AREAS_COUNT
};

char* D3D11ProfilerAreaNames[] = {
    "Shadow maps",
    "Shadow cubemaps",
    "Depth prepass",
    "Meshes",
    "Debug draw",
    "Atmosphere",
    "Postprocess",
};

u32 D3D11ProfilerAreaColors[] = {
    RGB(10, 150, 10),
    RGB(10, 100, 10),
    RGB(50, 50, 50),
    RGB(150, 10, 10),
    RGB(200, 180, 35),
    RGB(30, 50, 250),
    RGB(219, 61, 217),
};

static_assert(D3D11_PROFILE_AREAS_COUNT == ArrayCount(D3D11ProfilerAreaColors)
              && D3D11_PROFILE_AREAS_COUNT == ArrayCount(D3D11ProfilerAreaNames));

struct d3d11_state;
internal void D3D11_ProfilerBeginArea(d3d11_state*, d3d11_profiler_area);
internal void D3D11_ProfilerEndArea(d3d11_state*, d3d11_profiler_area);

struct d3d11_profile_block
{
    d3d11_profiler_area Area;
    d3d11_state* D3D11;
    
    d3d11_profile_block(d3d11_state* D, d3d11_profiler_area A) 
        : Area(A), D3D11(D)
    {
        D3D11_ProfilerBeginArea(D3D11, Area);
    }
    
    ~d3d11_profile_block() {
        D3D11_ProfilerEndArea(D3D11, Area);
    }
};

#define D3D11_PROFILE_BLOCK(D3D11, Area) d3d11_profile_block __D3D11ProfileBlock_##Area(D3D11, Area);

struct d3d11_profiler_frame
{
    ID3D11Query* QueryDisjoint;
    ID3D11Query* QueryBeginFrame;
    ID3D11Query* QueryEndFrame;
    
    ID3D11Query* BeginQueries[D3D11_PROFILE_AREAS_COUNT];
    ID3D11Query* EndQueries[D3D11_PROFILE_AREAS_COUNT];
};

struct d3d11_profiler
{
    d3d11_profiler_frame Frames[3]; //We do triple buffering of the frame data
    
    u32 CurrentFrame;   //Frame for which we are querying data
    
    b32 FrameIsValid;
    f32 FrameTime;
    f32 AreaTime[D3D11_PROFILE_AREAS_COUNT];
};

struct d3d11_state
{
    ID3D11Device* Device;
    ID3D11DeviceContext* Context;
    IDXGISwapChain* SwapChain;
    
    d3d11_monitor_info MonitorInfo;
    
    //Profiling
    d3d11_profiler Profiler;
    
    //Backbuffer
    ID3D11RenderTargetView* BackbufferRenderTargetView;
    D3D11_VIEWPORT BackbufferViewport;
    
    //Viewport
    d3d11_render_target DefaultRenderTarget;
    ID3D11DepthStencilView* DefaultDepthStencilView;
    ID3D11Texture2D* DefaultDepthStencilBuffer;
    D3D11_VIEWPORT Viewport;
    
    //For debug visualization
    ID3D11ShaderResourceView* DefaultDepthStencilResourceView;
        
    d3d11_common Common;
    d3d11_pbr_pipeline PBR;
    d3d11_precompute_pipeline Precompute;
    d3d11_shadow_pipeline Shadow;
    d3d11_atmosphere_pipeline Atmosphere;
    d3d11_sh_pipeline SH;
    d3d11_debug_pipeline Debug;
};