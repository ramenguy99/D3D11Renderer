internal asset_table_entry*
FindAsset(char* Name, asset_table Table, asset_type Type, char* Tag = "")
{
    for(u32 Index = 0; Index < Table.Count; Index++)
    {
        if(strncmp(Name, Table.Entries[Index].Name, ASSET_NAME_LENGTH) == 0 &&
           strncmp(Tag, Table.Entries[Index].Tag, ASSET_TAG_LENGTH) == 0)
        {
            if(Type != Table.Entries[Index].Type)
            {
                return 0;
            }
            return &Table.Entries[Index];
        }
    }
    
    return 0;
}

//Patches the children addresses by adding DataBegin to each Children pointer
internal void
LoadAssetJointTreeRecursively(mesh_joint* Joint, u8* DataBegin, u8* DataEnd)
{
    Assert((u64)Joint >= (u64)DataBegin && (u64)Joint < (u64)DataEnd);
    
    if(Joint->ChildrenCount)
    {
        Joint->Children = (mesh_joint*)(DataBegin + (u64)Joint->Children);
    }
    for(u32 Index = 0; Index < Joint->ChildrenCount; Index++)
    {
        LoadAssetJointTreeRecursively(&Joint->Children[Index], DataBegin, DataEnd);
    }
}

#define PATCH_ADDRESS(Address, Type, Begin, End) \
Address = (Type*)((u8*)Begin + (u64)Address); \
Assert((u64)Address <= (u64)End);

internal mesh_data
LoadMeshAsset(void* Data, u64 Size)
{
    mesh_data Result = {};
    
    asset_mesh* Asset = (asset_mesh*)Data;
    Result.Flags = (mesh_flag)Asset->Flags;
    Result.VerticesCount = Asset->VerticesCount;
    Result.IndicesCount = Asset->IndicesCount;
    Result.JointsCount = Asset->JointsCount;
    Result.AnimationsCount = Asset->AnimationsCount;
    
    u32 Vec4ArraySize = sizeof(vec4) * Asset->VerticesCount;
    u32 Vec3ArraySize = sizeof(vec3) * Asset->VerticesCount;
    u32 Vec2ArraySize = sizeof(vec2) * Asset->VerticesCount;
    
    u8* DataBegin = (u8*)Data;
    u32 PositionsOffset = Asset->VertexDataOffset;
    Result.Positions = (vec3*)(DataBegin + PositionsOffset);
    u32 NormalsOffset = PositionsOffset + Vec3ArraySize;
    Result.Normals = (vec3*)(DataBegin + NormalsOffset);
    u32 TangentsOffset = NormalsOffset + Vec3ArraySize;
    Result.Tangents = (vec3*)(DataBegin + TangentsOffset);
    u32 UVsOffset = TangentsOffset + Vec3ArraySize;
    Result.UVs = (vec2*)(DataBegin + UVsOffset);
    u32 IndicesOffset = UVsOffset + Vec2ArraySize;
    
    b32 HasAnimation = Asset->Flags & MESH_HAS_ANIMATION;
    if(HasAnimation)
    {
        u32 WeightsOffset = UVsOffset + Vec2ArraySize;
        Result.Weights = (vec4*)(DataBegin + WeightsOffset);
        u32 JointsOffset = WeightsOffset + Vec4ArraySize;
        Result.Joints = (ivec4*)(DataBegin + JointsOffset);
        IndicesOffset = JointsOffset + Vec4ArraySize;
    }
    
    Result.Indices = (u32*)(DataBegin + IndicesOffset);
    Assert(IndicesOffset + sizeof(u32) * Asset->IndicesCount <= Size);
    
    if(HasAnimation)
    {
        u32 RootJointOffset = IndicesOffset + sizeof(u32) * Asset->IndicesCount;
        Assert(RootJointOffset == Asset->JointsOffset);
        Result.RootJoint = (mesh_joint*)(DataBegin + RootJointOffset);
        LoadAssetJointTreeRecursively(Result.RootJoint, DataBegin, DataBegin + Size);
    
        u32 AnimationsOffset = RootJointOffset + sizeof(mesh_joint) * Asset->JointsCount;
        Assert(AnimationsOffset == Asset->AnimationsOffset);
        
        Result.Animations = (mesh_animation*)(DataBegin + AnimationsOffset);
        
        //Patch all the addressses with DataBegin
        for(u32 AnimIndex = 0; AnimIndex < Asset->AnimationsCount; AnimIndex++)
        {
            mesh_animation* Animation = &Result.Animations[AnimIndex];
            PATCH_ADDRESS(Animation->Joints, joint_animation, DataBegin, DataBegin + Size);
            for(u32 JointIndex = 0; JointIndex < Asset->JointsCount; JointIndex++)
            {
                joint_animation* Joint = &Animation->Joints[JointIndex];
                PATCH_ADDRESS(Joint->Keyframes, animation_keyframe, DataBegin, DataBegin + Size);
            }
        }
    }
    
    return Result;
}

#undef PATCH_ADDRESS

internal image_data
LoadImageAsset(void* Data, u64 Size)
{
    asset_image* Asset = (asset_image*)Data;
    Assert(Asset->Pitch == Asset->Width * Asset->BytesPerPixel);
    Assert(Size == sizeof(asset_image) + Asset->Height * Asset->Pitch);
    
    image_data Result = {};
    Result.Width = Asset->Width;
    Result.Height = Asset->Height;
    Result.BytesPerPixel = Asset->BytesPerPixel;
    Result.Pitch = Asset->Pitch;
    Result.Data = ((u8*)Data) + sizeof(asset_image);
    
    return Result;
}

internal cubemap_data
LoadCubemapAsset(void* Data, u64 Size)
{
    asset_cubemap* Asset = (asset_cubemap*)Data;
    
    cubemap_data Result = {};
    Result.Size = Asset->Size;
    Result.BytesPerPixel = Asset->BytesPerPixel;
    Result.Pitch = Asset->Pitch;
    Result.NumberOfMips = Asset->NumberOfMips ? Asset->NumberOfMips : 1;
    Result.Data = ((u8*)Data) + sizeof(asset_cubemap);
    
    return Result;
}