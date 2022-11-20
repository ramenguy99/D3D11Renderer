#include "camera.h"

internal void
FpCameraMove(fp_camera* Camera, camera_direction Direction, f32 DeltaTime)
{

    vec3 FrontMovement = Camera->Front / cosf(DegToRad(Camera->Pitch));
    switch(Direction)
    {
        case CAM_DIR_FORWARD:{
            Camera->Position.x += FrontMovement.x * DeltaTime * Camera->MoveVelocity;
            Camera->Position.y += FrontMovement.y * DeltaTime * Camera->MoveVelocity;
        } break;
        
        case CAM_DIR_BACKWARDS:{
            Camera->Position.x -= FrontMovement.x * DeltaTime * Camera->MoveVelocity;
            Camera->Position.y -= FrontMovement.y * DeltaTime * Camera->MoveVelocity;
        } break;
        
        case CAM_DIR_RIGHT:{
            Camera->Position.x += Camera->Right.x * DeltaTime * Camera->MoveVelocity;
            Camera->Position.y += Camera->Right.y * DeltaTime * Camera->MoveVelocity;
        } break;
        
        case CAM_DIR_LEFT:{
            Camera->Position.x -= Camera->Right.x * DeltaTime * Camera->MoveVelocity;
            Camera->Position.y -= Camera->Right.y * DeltaTime * Camera->MoveVelocity;
        } break;
        
        case CAM_DIR_UP:{
            Camera->Position.z += Camera->MoveVelocity * DeltaTime;
        } break;
        
        case CAM_DIR_DOWN:{
            Camera->Position.z -= Camera->MoveVelocity * DeltaTime;
        } break;
        
        default:{
            Assert(0);
        } break;
    }
}

internal void
FpCameraUpdateAngles(fp_camera* Camera, f32 MouseOffsetX, f32 MouseOffsetY)
{
    float32 YawVar   = MouseOffsetX * Camera->YawVelocity;
    float32 PitchVar = MouseOffsetY * Camera->PitchVelocity;
    Camera->Yaw   -= YawVar;
    Camera->Pitch -= PitchVar;
    Camera->Pitch = Clamp(Camera->Pitch, -89.0f, 89.0f);
}

internal void
FpCameraUpdateVectors(fp_camera* Camera)
{
    vec3 Front, Right, Up, WorldUp;
    Front.x = cosf(DegToRad(Camera->Yaw)) * cosf(DegToRad(Camera->Pitch));
    Front.y = sinf(DegToRad(Camera->Yaw)) * cosf(DegToRad(Camera->Pitch));
    Front.z = sinf(DegToRad(Camera->Pitch));
    Front = Normalize(Front);

    WorldUp = vec3(0.0f, 0.0f, 1.0f);
    Right = Normalize(Cross(Front, WorldUp));
    Up    = Normalize(Cross(Right, Front));

    Camera->Front = Front;
    Camera->Right = Right;
    Camera->Up = Up;

    Camera->View = Mat4LookAt(Camera->Position, Camera->Position +
                                Camera->Front, Camera->Up);
    
    Camera->Projection = Mat4Perspective(Camera->FOV, Camera->NearPlane, 
                                         Camera->FarPlane, Camera->AspectRatio);
}


internal void
FpCameraUpdateAnglesAndVectors(fp_camera* Camera, f32 MouseOffsetX, f32 MouseOffsetY)
{
    FpCameraUpdateAngles(Camera, MouseOffsetX, MouseOffsetY);
    FpCameraUpdateVectors(Camera);
}

internal vec3
GetRayFromNormalizedCoordinates(fp_camera* Camera, vec2 Coords)
{
    vec3 camPos = Camera->Position;
    
    vec3 rayFrom = camPos;
    vec3 rayForward = Camera->Front;
    float farPlane = 10000.f;
    rayForward = rayForward * farPlane;
    
    vec3 rightOffset;
    vec3 cameraUp= Camera->Up;
    vec3 vertical = cameraUp;
    
    vec3 hor = Camera->Right;
    
    hor = hor * (2.f * farPlane);
    vertical = hor * (2.f * farPlane);
    
    float aspect = Camera->AspectRatio;
    hor = hor * aspect;
    
    vec3 rayToCenter = rayForward;
    vec3 dHor = hor;
    vec3 dVert = vertical;
    
    vec3 rayTo = rayToCenter - 0.5f * hor + 0.5f * vertical;
    rayTo += Coords.x * dHor;
    rayTo -= Coords.y * dVert;
    
    return Normalize(rayTo);
}


//Coordinates in range [0-1], 0,0 is top-left corner
internal vec3
GetRayFromNormalizedCoordinates2(fp_camera* Camera, vec2 Coords)
{
    vec3 Eye = Camera->Position;
    vec3 Forward = Camera->Front;
    float t = tanf(DegToRad(Camera->FOV * 0.5f));
    vec3 Ver = Camera->Up * t;
    vec3 Hor = Camera->Right * t;
    Hor = Hor * Camera->AspectRatio;
    
    vec2 Offset = Coords * 2.0f - 1.0f;
    vec3 Ray = Forward + Offset.x * Hor - Offset.y * Ver;
    
    return Normalize(Ray);
}

