#include "mesh.h"

internal void
ReverseTriangleWinding(u32* Indices, u32 Count)
{
    Assert(Count % 3 == 0);
    for(u32 i = 0; i < Count; i += 3)
    {
        u32 Temp = Indices[i];
        Indices[i] = Indices[i + 1];
        Indices[i + 1] =  Temp;
    }
}

internal void
ReverseTriangleWinding(mesh_data* Mesh)
{
    ReverseTriangleWinding(Mesh->Indices, Mesh->IndicesCount);
}

internal void
ComputeMeshTangents(mesh_data* Mesh)
{
    if(!Mesh->Tangents) {
        Mesh->Tangents = (vec3*)ZeroAlloc(sizeof(vec3) * Mesh->VerticesCount);
    }
    
    vec3* Positions =  Mesh->Positions;
    vec3* Normals = Mesh->Normals;
    vec3* Tangents = Mesh->Tangents;
    vec2* UVs = Mesh->UVs;
    u32 VerticesCount = Mesh->VerticesCount;
    u32* Indices = Mesh->Indices;
    u32 IndicesCount = Mesh->IndicesCount;
    
    b32 IsStrip = Mesh->Flags & MESH_IS_STRIP;
    b32 HasIndices = !(Mesh->Flags & MESH_NO_INDICES);
    
    if(!HasIndices)
    {
        //If we don't have indices its as if we have 1 index per vertex
        IndicesCount = VerticesCount;
    }
    
    if(!IsStrip)
    {
        Assert(IndicesCount % 3 == 0);
    }
    
    vec3* Tan1 = (vec3*)ZeroAlloc(sizeof(vec3) * VerticesCount * 2);
    vec3* Tan2 = Tan1 + VerticesCount;
    
    for(u32 Index = 0;;)
    {
        u32 i0;
        u32 i1;
        u32 i2;
        if(!IsStrip)
        {
            i0 = Index + 0;
            i1 = Index + 1;
            i2 = Index + 2;
        }
        else
        {
            //If odd
            if(Index & 1)
            {
                i0 = Index + 0;
                i1 = Index + 1;
                i2 = Index + 2;
            }
            else
            {
                i0 = Index + 1;
                i1 = Index + 0;
                i2 = Index + 2;
            }
        }
        
        if(HasIndices)
        {
            i0 = Indices[i0];
            i1 = Indices[i1];
            i2 = Indices[i2];
        }
        
        vec3 Pos0 = Positions[i0];
        vec3 Pos1 = Positions[i1];
        vec3 Pos2 = Positions[i2];
        
        vec2 UV0 = UVs[i0];
        vec2 UV1 = UVs[i1];
        vec2 UV2 = UVs[i2];
        
        f32 x1 = Pos1.x - Pos0.x;
        f32 x2 = Pos2.x - Pos0.x;
        f32 y1 = Pos1.y - Pos0.y;
        f32 y2 = Pos2.y - Pos0.y;
        f32 z1 = Pos1.z - Pos0.z;
        f32 z2 = Pos2.z - Pos0.z;
        
        f32 s1 = UV1.x - UV0.x;
        f32 s2 = UV2.x - UV0.x;
        f32 t1 = UV1.y - UV0.y;
        f32 t2 = UV2.y - UV0.y;
        
        float epsilon = 1.0e-7f;
        float den = s1 * t2 - s2 * t1;
        
        // TODO: better way to handle this?
        if(den < epsilon && den > -epsilon) {
            den = epsilon;
        }
        
        float r = 1.0f / den;
        vec3 sdir = vec3((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
        vec3 tdir = vec3((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
        
        Tan1[i0] = Tan1[i0] + sdir;
        Tan1[i1] = Tan1[i1] + sdir;
        Tan1[i2] = Tan1[i2] + sdir;
        
        Tan2[i0] = Tan2[i0] + tdir;
        Tan2[i1] = Tan2[i1] + tdir;
        Tan2[i2] = Tan2[i2] + tdir;
        
        if(Index + 3 == IndicesCount) break;
        Index += IsStrip ? 1 : 3;
    }
    
    for(u32 Index = 0; Index < VerticesCount; Index++)
    {
        vec3 n = Normals[Index];
        vec3 t = Tan1[Index];
        
        // Gram-Schmidt orthogonalize
        t = t - n * Dot(n, t);
        if(t == vec3(0.0f))
        {
            t = vec3(1.0f, 0.0f, 0.0f);
        }
        else
        {
            t = Normalize(t);
        }
        
        Tangents[Index] = t;
        if(Dot(Cross(n, t), Tan2[Index]) < 0.0f)
        {
            Tangents[Index] = Negate(Tangents[Index]);
        }
        
        Assert(!isnan(Tangents[Index].x) &&
               !isnan(Tangents[Index].y) &&
               !isnan(Tangents[Index].z) );
    }
    
    Free(Tan1);
}


internal mesh_data
AllocCubeMesh(vec3 Size = vec3(1.0f, 1.0f, 1.0f))
{
    mesh_data Result = {};
    
    u32 VerticesCount = ArrayCount(GlobalCubePositions) / 3;
    
    Result.Positions = (vec3*)ZeroAlloc(sizeof(vec3) * VerticesCount);
    Result.Normals = (vec3*)ZeroAlloc(sizeof(vec3) * VerticesCount);
    Result.UVs = (vec2*)ZeroAlloc(sizeof(vec2) * VerticesCount);
    Result.VerticesCount = VerticesCount;
    
    Result.Flags = MESH_NO_INDICES;
    
    vec3* Positions = (vec3*)GlobalCubePositions;
    vec3* Normals = (vec3*)GlobalCubeNormals;
    vec2* UVs = (vec2*)GlobalCubeUVs;
    
    for(u32 i = 0; i < VerticesCount; i++)
    {
        Result.Positions[i] = HadamardProduct(Positions[i], Size);
        Result.Normals[i] = Normals[i];
        Result.UVs[i] = UVs[i];
    }
    
    
    Result.Tangents = (vec3*)ZeroAlloc(sizeof(vec3) * Result.VerticesCount);
    ComputeMeshTangents(&Result);
    
    return Result;
}

internal mesh_data
AllocSphereMesh(f32 Radius = 1.0f)
{
    mesh_data Result = {};
    
    Result.Flags = MESH_IS_STRIP;
    
    s32 X_SEGMENTS = 64;
    s32 Y_SEGMENTS = 64;
    
    u32 VerticesCount = (X_SEGMENTS + 1) * (Y_SEGMENTS + 1);
    Result.Positions = (vec3*)ZeroAlloc(sizeof(vec3) * VerticesCount);
    Result.Normals   = (vec3*)ZeroAlloc(sizeof(vec3) * VerticesCount);
    Result.UVs       = (vec2*)ZeroAlloc(sizeof(vec2) * VerticesCount);
    Result.VerticesCount = VerticesCount;
    
    u32 DataIndex = 0;
    for (s32 y = 0; y <= Y_SEGMENTS; ++y)
    {
        for (s32 x = 0; x <= X_SEGMENTS; ++x)
        {
            f32 xSegment = (f32)x / (f32)X_SEGMENTS;
            f32 ySegment = (f32)y / (f32)Y_SEGMENTS;
            f32 xPos = cosf(xSegment * 2.0f * PI) * sinf(ySegment * PI);
            f32 yPos = cosf(ySegment * PI);
            f32 zPos = sinf(xSegment * 2.0f * PI) * sinf(ySegment * PI);
            
            Assert(DataIndex < VerticesCount);
            
            Result.Positions[DataIndex] = vec3(xPos, yPos, zPos) * Radius;
            Result.Normals[DataIndex] = vec3(xPos, yPos, zPos);
            Result.UVs[DataIndex] = vec2(xSegment, ySegment);
            DataIndex++;
        }
    }
    
    u32 IndicesCount = 2 * (X_SEGMENTS + 1) * (Y_SEGMENTS);
    Result.Indices = (u32*)ZeroAlloc(sizeof(u32) * IndicesCount);
    Result.IndicesCount = IndicesCount;
        
    u32 IndicesIndex = 0;
    bool OddRow = false;
    for (s32 y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!OddRow) // even rows: y == 0, y == 2; and so on
        {
            for (s32 x = 0; x <=  X_SEGMENTS; ++x)
            {
                Result.Indices[IndicesIndex++] = y * (X_SEGMENTS + 1) + x;
                Result.Indices[IndicesIndex++] = (y + 1) * (X_SEGMENTS + 1) + x;
            }
        }
        else
        {
            for (s32 x = X_SEGMENTS; x >= 0; --x)
            {
                Result.Indices[IndicesIndex++] = (y + 1) * (X_SEGMENTS + 1) + x;
                Result.Indices[IndicesIndex++] = y * (X_SEGMENTS + 1) + x;
            }
        }
        OddRow = !OddRow;
    }
    
    Assert(IndicesIndex == IndicesCount);
    
    Result.Tangents = (vec3*)ZeroAlloc(sizeof(vec3) * Result.VerticesCount);
    ComputeMeshTangents(&Result);
    
    return Result;
}


internal void
UpdateJointStateRecursively(mesh_joint* Joint, mesh_animation* Animation, mat4 ParentTransform,
                            mat4* Joints, u32 Count, float Time)
{
    joint_animation* JointAnimation = &Animation->Joints[Joint->Id];
    if(JointAnimation->KeyframesCount == 0) return;
    
    Assert(Time >= 0.0f && Time <= JointAnimation->Keyframes[JointAnimation->KeyframesCount - 1].Time);
    u32 BeginIndex = 0;
    u32 EndIndex = 0;
    for(u32 Index = 1; Index < JointAnimation->KeyframesCount; Index++)
    {
        if(JointAnimation->Keyframes[Index].Time > Time)
        {
            EndIndex = Index;
            BeginIndex = Index - 1;
            break;
        }
    }
    
    animation_keyframe BeginKeyframe = JointAnimation->Keyframes[BeginIndex];
    animation_keyframe EndKeyframe = JointAnimation->Keyframes[EndIndex];
    
    float Progress = (Time - BeginKeyframe.Time) / (EndKeyframe.Time - BeginKeyframe.Time);
    vec3 Position = Lerp(BeginKeyframe.Position, EndKeyframe.Position, Progress);
    quaternion Rotation = Interpolate(BeginKeyframe.Rotation, EndKeyframe.Rotation, Progress);
    
    mat4 LocalTransform = Mat4Translation(Position) * QuaternionToMat4(Rotation);
    mat4 JointTransform = ParentTransform * LocalTransform;
    
    for(u32 Index = 0; Index < Joint->ChildrenCount; Index++)
    {
        UpdateJointStateRecursively(&Joint->Children[Index], Animation, JointTransform,
                                    Joints, Count, Time);
    }
    
    mat4 OutputTransform = JointTransform * Joint->InverseBindMatrix;
    Assert(Joint->Id <= Count);
    Joints[Joint->Id] = OutputTransform;
}

internal void
GetJointsFromAnimator(mesh_animator* Animator, mat4* Joints, u32 Count)
{
    Assert(Count >= Animator->JointsCount);
    UpdateJointStateRecursively(Animator->RootJoint, &Animator->Animations[0], Mat4Identity(),
                                Joints, Count, Animator->Time);
}

internal void
UpdateAnimator(mesh_animator* Animator, float Delta)
{
    Animator->Time += Delta;
    mesh_animation* Animation = Animator->Animations;
    
    if(Animator->Time > Animation->Duration)
    {
        Animator->Time -= Animation->Duration;
    }
}

internal void
FreeMesh(mesh_data* Mesh)
{
    for(u32 i = 0; i < ArrayCount(Mesh->VertexData); i++)
    {
        Free(Mesh->VertexData[i]);
    }
    Free(Mesh->Indices);
    // TODO: Free animation data
    
    *Mesh = {};
}