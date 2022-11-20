enum camera_direction
{
    CAM_DIR_NONE,
    CAM_DIR_FORWARD,
    CAM_DIR_BACKWARDS,
    CAM_DIR_RIGHT,
    CAM_DIR_LEFT,
    CAM_DIR_UP,
    CAM_DIR_DOWN,
};

struct fp_camera
{
    vec3 Position;
    vec3 Front;
    vec3 Right;
    vec3 Up;
    
    float32 Pitch;
    float32 Yaw;
    
    f32 MoveVelocity;
    f32 PitchVelocity;
    f32 YawVelocity;
    
    f32 FOV;
    f32 AspectRatio;
    f32 NearPlane;
    f32 FarPlane;
    
    mat4 View;
    mat4 Projection;
};

struct camera
{
    vec3 Position;
    vec3 Front;
    vec3 Right;
    vec3 Up;
    
    f32 FOV;
    f32 AspectRatio;
    f32 NearPlane;
    f32 FarPlane;
};

