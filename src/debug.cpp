#include "debug.h"

internal void
BeginDebugFrame()
{
    Debug.LinesCount = 0;
    Debug.TrianglesCount = 0;
}


internal void
DebugLine(vec3 A, vec3 B, u32 Color)
{
    Assert(Debug.LinesCount < MAX_DEBUG_LINES);
    Debug.Lines[Debug.LinesCount++] = {A, B, Color};
}

internal void
DebugTriangle(vec3 A, vec3 B, vec3 C, u32 Color)
{
    Assert(Debug.LinesCount < MAX_DEBUG_LINES);
    Debug.Triangles[Debug.TrianglesCount++] = {A, B, C, Color};
}

internal void
DebugQuad(vec3 A, vec3 B, vec3 C, vec3 D, u32 Color)
{
    DebugTriangle(A, B, C, Color);
    DebugTriangle(B, C, D, Color);
}

internal void ComputeFrustumVertices(mat4, vec3*);

internal void
DebugFrustum(mat4 Matrix, u32 Color)
{
    vec3 Points[8];
    ComputeFrustumVertices(Matrix, Points);
    
    DebugLine(Points[0], Points[1], Color);
    DebugLine(Points[0], Points[2], Color);
    DebugLine(Points[1], Points[3], Color);
    DebugLine(Points[2], Points[3], Color);
    
    DebugLine(Points[4 + 0], Points[4 + 1], Color);
    DebugLine(Points[4 + 0], Points[4 + 2], Color);
    DebugLine(Points[4 + 1], Points[4 + 3], Color);
    DebugLine(Points[4 + 2], Points[4 + 3], Color);
    
    DebugLine(Points[0], Points[4 + 0], Color);
    DebugLine(Points[1], Points[4 + 1], Color);
    DebugLine(Points[2], Points[4 + 2], Color);
    DebugLine(Points[3], Points[4 + 3], Color);
}

internal void
DebugPoint(vec3 P, u32 Color)
{
    f32 S = 0.1f;
    DebugLine(P - vec3(0, 0, S), P + vec3(0, 0, S), Color);
    DebugLine(P - vec3(0, S, 0), P + vec3(0, S, 0), Color);
    DebugLine(P - vec3(S, 0, 0), P + vec3(S, 0, 0), Color);
}

internal void
DebugAABB(vec3 Min, vec3 Max, u32 Color)
{
    vec3 Dim = Max - Min;
    
    //Bottom face
    DebugLine(Min, Min + vec3(Dim.x, 0, 0), Color);
    DebugLine(Min, Min + vec3(0, Dim.y, 0), Color);
    DebugLine(Min + vec3(0, Dim.y, 0), Min + vec3(Dim.x, Dim.y, 0), Color);
    DebugLine(Min + vec3(Dim.x, 0, 0), Min + vec3(Dim.x, Dim.y, 0), Color);
    
    
    //Top face
    DebugLine(Min + vec3(0, 0, Dim.z),     Min + vec3(Dim.x, 0, Dim.z), Color);
    DebugLine(Min + vec3(0, 0, Dim.z),     Min + vec3(0, Dim.y, Dim.z), Color);
    DebugLine(Min + vec3(0, Dim.y, Dim.z), Max, Color);
    DebugLine(Min + vec3(Dim.x, 0, Dim.z), Max, Color);
    
    //Vertical lines
    DebugLine(Min,                         Min + vec3(0, 0, Dim.z), Color); 
    DebugLine(Min + vec3(Dim.x, Dim.y, 0), Max, Color);
    DebugLine(Min + vec3(0, Dim.y, 0),     Min + vec3(0, Dim.y, Dim.z), Color);
    DebugLine(Min + vec3(Dim.x, 0, 0),     Min + vec3(Dim.x, 0, Dim.z), Color);
}

internal void
DebugPlane(vec3 N, f32 D, u32 PlaneColor, u32 NormalColor)
{
    vec3 C = N * -D;
    DebugLine(C, C + N, NormalColor);
    
    vec3 P = Normalize(Cross(N, vec3(10, 1, -5))) * 10;
    vec3 O = Normalize(Cross(P, N)) * 10;
    DebugQuad(C + P + O, C + P - O, C - P + O, C - P - O, PlaneColor);
}

internal void
EndDebugFrame()
{
    
}