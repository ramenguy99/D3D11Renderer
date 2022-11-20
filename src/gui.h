struct gpu_frame_slider {
    struct {
        f32 Time;
        f32 AreaTime[D3D11_PROFILE_AREAS_COUNT];
    } Frames[256];
    
    u32 CurrentFrameIndex = 0;
    b32 Paused;
};

global_variable gpu_frame_slider GpuFrameSlider;
