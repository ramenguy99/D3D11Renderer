#ifndef DEBUG_H
#define DEBUG_H

#define MAX_DEBUG_LINES (1024 * 1024)
#define MAX_DEBUG_TRIANGLES (1024 * 1024)

struct debug_line
{
    vec3 A;
    vec3 B;
    u32 Color;
};

struct debug_triangle
{
    vec3 A;
    vec3 B;
    vec3 C;
    u32 Color;
};

struct debug_data {
    debug_line Lines[MAX_DEBUG_LINES];
    debug_triangle Triangles[MAX_DEBUG_TRIANGLES];
    u32 LinesCount;
    u32 TrianglesCount;
};

global_variable debug_data Debug;


#endif //DEBUG_H
