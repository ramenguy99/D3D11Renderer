#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <float.h>
#include <stdint.h>
#include <math.h>

#include "defines.h"
#define ENABLE_GLOBAL_RAND_STATE
#include "math/math.cpp"
#include "math/math_vec.cpp"
#include "math/math_mat.cpp"
#include "math/math_quaternion.cpp"

// IMPORTANT: Those must match the defines in the pbr_common shader
#define MAX_DIRECTIONAL_LIGHTS_COUNT 4
#define MAX_POINT_LIGHTS_COUNT 4

#include "startup_timestamps.cpp"
#include "debug.cpp"
#include "geometry.cpp"
#include "bounding_volumes.cpp"
#include "mesh.cpp"
#include "image.cpp"
#include "atmosphere.cpp"

#include "asset_file.h"
#include "asset_loader.cpp"
#include "direct3d11.cpp"
#include "imgui_d3d11.cpp"
#include "inspector.cpp"
#include "win32.cpp"

typedef d3d11_mesh mesh_gpu;
typedef d3d11_texture texture;
typedef d3d11_cubemap cubemap;
typedef d3d11_shadow_map shadow_map;
typedef d3d11_shadow_cubemap shadow_cubemap;
typedef d3d11_light_probe light_probe;
typedef d3d11_atmosphere atmosphere;

#include "camera.cpp"
#include "scene.cpp"
#include "direct3d11_scene.cpp"
#include "gui.cpp"
#include "spherical_harmonics.cpp"


int
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd)
{
    STARTUP_BEGIN;

    // Create window
    s32 Width = 1600;
    s32 Height = 900;
    HWND Window = Win32_CreateWindow("Editor", Width, Height);
    Win32.WindowResized = false; //Set resized to false after the first default resize on creation

    STARTUP_TIMESTAMP(WINDOW);

    // Load asset file
    HANDLE AssetFile = CreateFile("../res/data.asset", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    Assert(AssetFile != INVALID_HANDLE_VALUE);
    DWORD BytesRead = 0;
    asset_file_header AssetHeader = {};
    ReadFile(AssetFile, &AssetHeader, sizeof(asset_file_header), &BytesRead, 0);
    Assert(BytesRead == sizeof(asset_file_header));

    asset_table AssetTable = {};
    AssetTable.Count = AssetHeader.TableEntryCount;
    u64 AssetTableSize = sizeof(asset_table_entry) * AssetTable.Count;
    AssetTable.Entries = (asset_table_entry*)ZeroAlloc(AssetTableSize);
    ReadFile(AssetFile, AssetTable.Entries, (u32)AssetTableSize, &BytesRead, 0);
    Assert(BytesRead == AssetTableSize);

    STARTUP_TIMESTAMP(ASSET_FILE);

    // Initialize D3D11 Renderer
    d3d11_state D3D11 = D3D11_Init(Window, Width, Height);

    // Load baked BRDF texture from asset file
    D3D11.PBR.BRDFTexture = LoadTextureFromAssetFile(D3D11.Device, AssetFile, AssetTable, "brdf", DXGI_FORMAT_R16G16_FLOAT);

    // Track some shaders
    PushTrackedShader(L"../src/shaders/pbr_pixel.hlsl", (void**)&D3D11.PBR.PixelShader, TRACKED_SHADER_PIXEL);
    PushTrackedShader(L"../src/shaders/bruneton/atmosphere_render.hlsl", (void**)&D3D11.Atmosphere.RenderAtmosphereShader, TRACKED_SHADER_PIXEL);

    // Initialize IGUI
    ImguiInit(Window, &D3D11);

    STARTUP_TIMESTAMP(IMGUI);

    // Get refresh rate and init counters
    f32 SecondsPerFrame = 1.0f / D3D11.MonitorInfo.RefreshRate;
    f32 SecondsElapsed = SecondsPerFrame; //The first frame we step by the expected frame time
    s64 CounterBegin = Win32_GetCurrentCounter();
    s64 CounterEnd = CounterBegin;


    // Load helmet model from asset file
    mesh_data HelmetMesh = LoadMeshFromAssetFile(AssetFile, AssetTable, "helmet");

    // Fixup UVs and triangle winding
    for(u32 i = 0; i < HelmetMesh.VerticesCount; i++) {
        HelmetMesh.UVs[i].y += 1.0f;
    }
    ReverseTriangleWinding(&HelmetMesh);

    mesh_gpu HelmetGpuMesh = D3D11_LoadMesh(D3D11.Device, &HelmetMesh);

    STARTUP_TIMESTAMP(MESHES);

    // Load material from asset file
    material HelmetMaterial = LoadMaterialFromAssetFile(D3D11.Device, AssetFile, AssetTable, "helmet");
    STARTUP_TIMESTAMP(MATERIALS);

    // Create shadow maps
    ivec2 ShadowMapSize = ivec2(2048, 2048);
    shadow_map ShadowMap = D3D11_CreateShadowMap(D3D11.Device, ShadowMapSize.x, ShadowMapSize.y);
    PushTrackedTexture(ShadowMap.ResourceView, ShadowMapSize, "Shadow map", TRACKED_TEXTURE_2D);

    s32 ShadowCubemapSize = 1024;
    shadow_cubemap ShadowCubemap0 = D3D11_CreateShadowCubemap(D3D11.Device, ShadowCubemapSize);
    PushTrackedTexture(ShadowCubemap0.ResourceView, ivec2(ShadowCubemapSize), "Shadow cubemap", TRACKED_TEXTURE_CUBEMAP);

    STARTUP_TIMESTAMP(SHADOW_MAPS);

    // Load baked light probe from asset file
    light_probe LightProbe = LoadLightProbeFromAssetFile(D3D11.Device, AssetFile, AssetTable, "harbor");
    PushTrackedTexture(LightProbe.Base.ResourceView, ivec2(HDR_CUBEMAP_SIZE), "Environment - base", TRACKED_TEXTURE_CUBEMAP);
    PushTrackedTexture(LightProbe.Irradiance.ResourceView, ivec2(IRRADIANCE_CUBEMAP_SIZE), "Environment - irradiance", TRACKED_TEXTURE_CUBEMAP);
    PushTrackedTexture(LightProbe.Specular.ResourceView, ivec2(SPECULAR_CUBEMAP_SIZE), "Environment - specular", TRACKED_TEXTURE_CUBEMAP);

    STARTUP_TIMESTAMP(LIGHT_PROBE);

    // Load atmosphere from atmo file
    atmosphere_parameters EarthAtmosphereParameters = GetEarthAtmosphereParameters();
    atmosphere EarthAtmosphere = {};
    EarthAtmosphere = D3D11_LoadAtmosphereFromFile(&D3D11, "../res/earth.atmo");

    STARTUP_TIMESTAMP(ATMOSPHERE);

    // Track atmosphere textures
    ivec2 TransmittanceSize = ivec2(EarthAtmosphere.Transmittance.Width, EarthAtmosphere.Transmittance.Height);
    PushTrackedTexture(EarthAtmosphere.Transmittance.ResourceView, TransmittanceSize, "Earth - Transmittance", TRACKED_TEXTURE_2D);

    d3d11_render_target DeltaIrradiance = D3D11.Atmosphere.DeltaIrradiance;
    ivec2 DeltaIrradianceSize = ivec2(DeltaIrradiance.Width, DeltaIrradiance.Height);
    PushTrackedTexture(EarthAtmosphere.Irradiance.ResourceView, DeltaIrradianceSize, "Earth - Irradiance", TRACKED_TEXTURE_2D);

    d3d11_render_target_3d Scattering = EarthAtmosphere.Scattering;
    ivec2 ScatteringSize = ivec2(Scattering.Width, Scattering.Height);
    PushTrackedTexture(Scattering.ResourceView, ScatteringSize, "Earth - Scattering", TRACKED_TEXTURE_3D, Scattering.Depth);

    // Initialize scene
    scene* Scene = (scene*)ZeroAlloc(sizeof(scene));
    Scene->Probe = LightProbe;
    Scene->Atmosphere = EarthAtmosphere;
    Scene->SunDirection = Normalize(vec3(-1, 1, 1));
    Scene->SunIlluminanceColor = vec3(1.0f);
    Scene->SunIlluminanceScale = 10.0f;
    AddMaterial(Scene, &HelmetMaterial);


    mesh* Helmet = AddMesh(Scene, "Helmet", &HelmetMesh, &HelmetGpuMesh, 0, true);
    Helmet->Rotation = vec3(90, 0, 0);
    Helmet->Position = vec3(0, 0, 5);

    AddPointLight(Scene, vec3(-2, -2, 5), 30.0f, vec3(100, 100, 100), ShadowCubemap0);

    // Initialize camera
    fp_camera Camera = {};
    Camera.Position = vec3(0, -2, 5);
    Camera.Pitch = 0.0f;
    Camera.Yaw = 90.0f;
    Camera.MoveVelocity = 5.0f;
    Camera.PitchVelocity = 0.1f;
    Camera.YawVelocity = 0.1f;
    Camera.FOV = 60.0f;
    Camera.AspectRatio = 16.0f/9.0f;

#if NEAR_FAR_PLANE_INVERSION
    Camera.NearPlane = 1000.0f;
    Camera.FarPlane = 0.1f;
#else
    Camera.NearPlane = 0.1f;
    Camera.FarPlane = 1000.0f;
#endif

    FpCameraUpdateAnglesAndVectors(&Camera, 0, 0);
    mat4 CameraBegin = Camera.Projection * Camera.View;

    STARTUP_TIMESTAMP(SCENE);

    // Initialize mouse position
    vec2 LastMousePos = Win32_GetMousePosition(Window);
    vec2 MousePos;

    // Spherical harmonics tests
    PushTrackedShader(L"../src/shaders/sh_test.hlsl", (void**)&D3D11.SH.TestPixelShader, TRACKED_SHADER_PIXEL);
    PushTrackedTexture(0, ivec2(0), "SH Test", TRACKED_SH_MAP);
    ComputeSphericalHarmonics(AssetFile, AssetTable, "harbor");


    // Main loop
    while(!Win32.WindowQuit)
    {
        // Debug
        BeginDebugFrame();
        CheckShadersForUpdate(D3D11.Device);

        // Process window events
        MSG Message;
        while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        // Resize D3D11 buffers
        if(Win32.WindowResized)
        {
            D3D11_ResizeBackbuffer(&D3D11, Win32.WindowSize);
            Win32.WindowResized = false;
        }

        // Minimal keyboard input
        b32 InputW = GetKeyState('W') & (1 << 15);
        b32 InputA = GetKeyState('A') & (1 << 15);
        b32 InputS = GetKeyState('S') & (1 << 15);
        b32 InputD = GetKeyState('D') & (1 << 15);
        b32 InputE = GetKeyState('E') & (1 << 15);
        b32 InputQ = GetKeyState('Q') & (1 << 15);
        b32 InputR = GetKeyState('R') & (1 << 15);
        b32 InputSpace = GetKeyState(VK_SPACE) & (1 << 15);

        if(InputW) FpCameraMove(&Camera, CAM_DIR_FORWARD, SecondsElapsed);
        if(InputA) FpCameraMove(&Camera, CAM_DIR_LEFT, SecondsElapsed);
        if(InputS) FpCameraMove(&Camera, CAM_DIR_BACKWARDS, SecondsElapsed);
        if(InputD) FpCameraMove(&Camera, CAM_DIR_RIGHT, SecondsElapsed);
        if(InputE) FpCameraMove(&Camera, CAM_DIR_UP, SecondsElapsed);
        if(InputQ) FpCameraMove(&Camera, CAM_DIR_DOWN, SecondsElapsed);

        if(InputSpace) GpuFrameSlider.Paused = true;

        // Minimal mouse input
        vec2 MouseOffset = vec2(0, 0);
        MousePos = Win32_GetMousePosition(Window);
        if(GetKeyState(VK_RBUTTON) & (1 << 15))
        {
            MouseOffset = MousePos - LastMousePos;
        }
        LastMousePos = MousePos;
        FpCameraUpdateAnglesAndVectors(&Camera, MouseOffset.x, MouseOffset.y);

        // Update camera data
        Scene->Projection = Camera.Projection;
        Scene->View = Camera.View;
        Scene->ViewPosition = Camera.Position;
        Scene->CameraForward = Camera.Front;
        Scene->CameraNear = Camera.NearPlane;
        Scene->CameraFar = Camera.FarPlane;

        float t = tanf(DegToRad(Camera.FOV * 0.5f));
        vec3 Ver = Camera.Up * t;
        vec3 Hor = Camera.Right * t;
        Hor = Hor * Camera.AspectRatio;
        Scene->PerspectiveHor = Hor;
        Scene->PerspectiveVer = Ver;

        // GUI
        ImguiBeginFrame();

        if(InspectorData.ShowDemoWindow)
        {
            ImGui::ShowDemoWindow(&InspectorData.ShowDemoWindow);
        }

        ivec2 ViewportSize = Win32.WindowSize;
        if(InspectorData.ShowEditor)
        {
            DrawEditor(&D3D11, Scene, AssetTable, SecondsElapsed);
        }
        else
        {
            if(Win32.WindowSize != ivec2(D3D11.DefaultRenderTarget.Width, D3D11.DefaultRenderTarget.Height))
            {
                D3D11_ResizeDefaultRenderTargets(&D3D11, Win32.WindowSize);
            }
            DrawStats(vec2(10, 10), SecondsElapsed, D3D11.Profiler.FrameTime);
        }

        // Clear the back buffer
        D3D11_Clear(&D3D11, vec4(0.8f, 0.8f, 0.8f, 1.0f));

        // We start profiling right after the clear because the clear on the backbuffer
        // stalls if vsync is enabled, it appears that our driver does 2 frames of buffering
        // and doesn't let us clear the 3rd frame (which is frame number 0 for the second time)
        // if it hasn't been displayed yet
        D3D11_ProfilerBeginFrame(&D3D11);


        // Main scene rendering
        D3D11_DrawScene(&D3D11, Scene);

        // Debug rendering
        {
            D3D11_PROFILE_BLOCK(&D3D11, D3D11_PROFILE_DEBUG_DRAW);
            D3D11_DebugDraw(&D3D11, Camera.Projection * Camera.View);
        }

        // GPU Profiler
        D3D11_ProfilerEndFrame(&D3D11);
        UpdateGpuFrameSlider(&D3D11);

        // Blit the intermediate target into the back buffer if there is no editor open
        if(!InspectorData.ShowEditor)
        {
            ID3D11Texture2D* BackBuffer;
            HRESULT Result = D3D11.SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
            Assert(Result == S_OK);
            D3D11.Context->CopyResource(BackBuffer, D3D11.DefaultRenderTarget.Texture);
            BackBuffer->Release();
        }

        // Render GUI
        ImguiRender(&D3D11);

        // Present
        D3D11_Present(&D3D11, InspectorData.VSyncEnabled);

        // Finilize frame
        EndDebugFrame();

        // Update counters
        CounterEnd = Win32_GetCurrentCounter();
        SecondsElapsed = Win32_GetSecondsElapsed(CounterBegin, CounterEnd);
        CounterBegin = CounterEnd;

        //NOTE: Cap seconds elapsed to avoid super long timesteps if debugging
        if(SecondsElapsed > 1.0f)
        {
            SecondsElapsed = SecondsPerFrame;
        }

    }

    // Cleanup ImGui Context, saves the .ini file
    ImguiCleanup();

    return 0;
}
