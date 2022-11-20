enum startup_action
{
    STARTUP_WINDOW,
    STARTUP_ASSET_FILE,
    STARTUP_D3D11_DEVICE,
    STARTUP_D3D11_DEFAULT_TARGETS,
    STARTUP_D3D11_COMMON,
    STARTUP_D3D11_PBR,
    STARTUP_D3D11_PRECOMPUTE_LIGHT_PROBES,
    STARTUP_D3D11_SHADOW_MAPS,
    STARTUP_D3D11_PRECOMPUTE_ATMOSPHERE,
    STARTUP_D3D11_SPHERICAL_HARMONICS,
    STARTUP_D3D11_DEBUG,
    STARTUP_D3D11_PROFILER,
    STARTUP_IMGUI,
    STARTUP_MESHES,
    STARTUP_MATERIALS,
    STARTUP_SHADOW_MAPS,
    STARTUP_LIGHT_PROBE,
    STARTUP_ATMOSPHERE,
    STARTUP_SCENE,
    
    STARTUP_ACTIONS_COUNT,
};

char* StartupActionNames[] = {
    "Window",
    "Asset File",
    "D3D11 - Device and Swapchain",
    "D3D11 - Common state",
    "D3D11 - Default render targets and depth",
    "D3D11 - PBR pipeline",
    "D3D11 - Light probe precomputation",
    "D3D11 - Shadow maps pipeline",
    "D3D11 - Atmosphere precomputation",
    "D3D11 - Spherical harmonics",
    "D3D11 - Debug pipeline",
    "D3D11 - Profiler",
    "Imgui",
    "Meshes",
    "Materials",
    "Light probe",
    "Atmosphere",
    "Shadow Maps",
    "Scene setup",
};

struct startup_timestamps
{
    u64 Begin;
    u64 Timestamps[STARTUP_ACTIONS_COUNT];
};

static_assert(ArrayCount(StartupActionNames) == STARTUP_ACTIONS_COUNT);

global_variable startup_timestamps StartupTimestamps;

internal s64 Win32_GetCurrentCounter();

#define STARTUP_BEGIN StartupTimestamps.Begin = Win32_GetCurrentCounter()
#define STARTUP_TIMESTAMP(x) StartupTimestamps.Timestamps[STARTUP_##x] = Win32_GetCurrentCounter()
