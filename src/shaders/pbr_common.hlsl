// IMPORTANT: Those must match the defines in main.cpp
#define MAX_DIRECTIONAL_LIGHTS_COUNT 4
#define MAX_POINT_LIGHTS_COUNT 4

struct pixel_input
{
    vec4 Position : SV_POSITION;
    
    vec3 WorldPos : POSITION0;
    vec4 ShadowPos[MAX_DIRECTIONAL_LIGHTS_COUNT] : POSITION1;
    vec2 TexCoord : TEXCOORD0;
    vec3 Normal : NORMAL;
    vec3 Tangent : TANGENT;
};