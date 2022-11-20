struct plane
{
    vec3 Normal;
    f32 D;
};

union aabb
{
    struct {
        vec3 Min;
        vec3 Max;
    };
    
    vec3 Points[2];
};

struct frustum
{
    vec3 Vertices[8];
    plane Planes[6];
};

inline b32
IsPointInAABB(vec3 P, aabb AABB)
{
    return
        P.x <= AABB.Max.x &&
        P.y <= AABB.Max.y &&
        P.z <= AABB.Max.z &&
        P.x >= AABB.Min.x &&
        P.y >= AABB.Min.y &&
        P.z >= AABB.Min.z;
}

inline b32
IsAABBInInnerHalfspace(aabb A, plane P)
{
    vec3 N = P.Normal;
    
    u32 X = N.x >= 0.0f;
    u32 Y = N.y >= 0.0f;
    u32 Z = N.z >= 0.0f;
    
    vec3 V = vec3(A.Points[X].x, A.Points[Y].y, A.Points[Z].z);
    b32 Result = Dot(V, N) >= -P.D;
    
    return Result;
}

inline b32
IsAABBInsideFrustum(aabb A, plane* Planes)
{
    for(u32 i = 0; i < 6; i++)
    {
        if(!IsAABBInInnerHalfspace(A, Planes[i]))
            return false;
    }
    
    return true;
}

internal aabb
ComputeAABB(vec3* Positions, u32 Count)
{
    aabb Result = {};
    Result.Min = vec3(FLT_MAX);
    Result.Max = vec3(-FLT_MAX);
    for(u32 Index = 0; Index < Count; Index++)
    {
        vec3 P = Positions[Index];
        if(P.x > Result.Max.x) Result.Max.x = P.x;
        if(P.y > Result.Max.y) Result.Max.y = P.y;
        if(P.z > Result.Max.z) Result.Max.z = P.z;
        
        if(P.x < Result.Min.x) Result.Min.x = P.x;
        if(P.y < Result.Min.y) Result.Min.y = P.y;
        if(P.z < Result.Min.z) Result.Min.z = P.z;
    }
    
    return Result;
}

internal aabb
ComputeAABB(vec3* Positions, u32 Count, mat3 Transform, vec3 Offset)
{
    aabb Result = {};
    Result.Min = vec3(FLT_MAX);
    Result.Max = vec3(-FLT_MAX);
    for(u32 Index = 0; Index < Count; Index++)
    {
        vec3 P = Transform * Positions[Index] + Offset;
        if(P.x > Result.Max.x) Result.Max.x = P.x;
        if(P.y > Result.Max.y) Result.Max.y = P.y;
        if(P.z > Result.Max.z) Result.Max.z = P.z;
        
        if(P.x < Result.Min.x) Result.Min.x = P.x;
        if(P.y < Result.Min.y) Result.Min.y = P.y;
        if(P.z < Result.Min.z) Result.Min.z = P.z;
    }
    
    return Result;
}

internal plane
NormalizedPlane(vec4 V)
{
    plane Result = {};
    
    vec3 N = vec3(V);
    f32 Len = Length(N);
    
    Result.Normal = N / Len;
    Result.D = V.w / Len;
    
    return Result;
}

//Computes the 6 planes that enclose the frustum, the normal of the plane always points
//towards the inside of the frustum
internal void
ComputeFrustumPlanes(mat4 M, plane* OutPlanes)
{
    M = Mat4Transpose(M);
    
    // Left Frustum Plane
    // Add first column of the matrix to the fourth column
    vec4 Left = M.Columns[3] + M.Columns[0];
    OutPlanes[0] = NormalizedPlane(Left);
    
    // Right Frustum Plane
    // Subtract first column of matrix from the fourth column
    vec4 Right = M.Columns[3] - M.Columns[0];
    OutPlanes[1] = NormalizedPlane(Right);
    
    // Top Frustum Plane
    // Subtract second column of matrix from the fourth column
    vec4 Top = M.Columns[3] - M.Columns[1];
    OutPlanes[2] = NormalizedPlane(Top);
    
    // Bottom Frustum Plane
    // Add second column of the matrix to the fourth column
    vec4 Bottom = M.Columns[3] + M.Columns[1];
    OutPlanes[3] = NormalizedPlane(Bottom);
    
    // Near Frustum Plane
    // We could add the third column to the fourth column to get the near plane,
    // but we don't have to do this because the third column IS the near plane
    vec4 Near = M.Columns[3] + M.Columns[2] ;
    OutPlanes[4] = NormalizedPlane(Near);
    
    // Far Frustum Plane
    // Subtract third column of matrix from the fourth column
    vec4 Far = M.Columns[3] - M.Columns[2];
    OutPlanes[5] = NormalizedPlane(Far);
}   

internal void
ComputeFrustumVertices(mat4 Matrix, vec3* Frustum)
{
    vec4 FrustumFront[4] = {
        vec4(-1.0f, 1.0f, 0.0f, 1.0f),
        vec4(1.0f, 1.0f, 0.0f, 1.0f),
        vec4(-1.0f, -1.0f, 0.0f, 1.0f),
        vec4(1.0f, -1.0f, 0.0f, 1.0f),
    };
    vec4 FrustumBack[4] = {
        vec4(-1.0f, 1.0f, 1.0f, 1.0f),
        vec4(1.0f, 1.0f, 1.0f, 1.0f),
        vec4(-1.0f, -1.0f, 1.0f, 1.0f),
        vec4(1.0f, -1.0f, 1.0f, 1.0f),
    };
    
    mat4 Inverse = Mat4Inverse(Matrix);
    
    for(u32 Index = 0; Index < 4; Index++)
    {
        vec4 A = Inverse * FrustumFront[Index];
        Frustum[Index] = vec3(A) / A.w;
        
        vec4 B = Inverse * FrustumBack[Index];
        Frustum[Index + 4] = vec3(B) / B.w;
    }
}

internal frustum
FrustumFromMatrix(mat4 Matrix)
{
    frustum Result;
    ComputeFrustumPlanes(Matrix, Result.Planes);
    ComputeFrustumVertices(Matrix, Result.Vertices);
    
    return Result;
}

internal b32
FrustumFrustumIntersection(frustum* A, frustum* B)
{
    //A planes vs B vertices
    for(u32 i = 0; i < 6; i++)
    {
        plane P = A->Planes[i];
        
        bool AnyVertexInPositiveSide = false;
        for(u32 j = 0; j < 8; j++)
        {
            AnyVertexInPositiveSide |= Dot(P.Normal, B->Vertices[j]) + P.D > 0;
        }
        
        if(!AnyVertexInPositiveSide) 
            return false;
    }
    
    //A vertices vs B planes
    for(u32 i = 0; i < 6; i++)
    {
        plane P = B->Planes[i];
        
        bool AnyVertexInPositiveSide = false;
        for(u32 j = 0; j < 8; j++)
        {
            AnyVertexInPositiveSide |= Dot(P.Normal, A->Vertices[j]) + P.D > 0;
        }
        
        if(!AnyVertexInPositiveSide) 
            return false;
    }
    
    return true;
}
