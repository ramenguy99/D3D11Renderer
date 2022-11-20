#define MAX_MESH_JOINTS 64

struct mesh_joint
{
    char* Name;
    u64 Id;
    mat4 InverseBindMatrix;
    
    mesh_joint* Children;
    u32 ChildrenCount;
};

struct animation_keyframe
{
    float Time;
    vec3 Position;
    quaternion Rotation;
};

struct joint_animation
{
    animation_keyframe* Keyframes;
    u32 KeyframesCount;
};

struct mesh_animation
{
    //One for each joint, joints refer to this by their Id element
    //there are Mesh.JointsCount elements in here
    joint_animation* Joints;
    float Duration; //Total duration of animation
};

enum mesh_flag
{
    MESH_HAS_ANIMATION = 1 << 0,
    MESH_IS_STRIP      = 1 << 1,
    MESH_NO_INDICES    = 1 << 2,
};

struct mesh_data
{
    union
    {
        struct
        {
            vec3* Positions;
            vec3* Normals;
            vec3* Tangents;
            vec2* UVs;
            
            //Available only if MESH_HAS_ANIMATION
            vec4* Weights;
            ivec4* Joints; //Indices into the array of joints that affect the vertex
        };
        
        void* VertexData[6];
    };
    
    mesh_flag Flags;
    u32 VerticesCount;
    
    //Available only if no MESH_NO_INDICES
    u32* Indices;
    u32 IndicesCount;
    
    //Available only if MESH_HAS_ANIMATION
    mesh_joint* RootJoint; //Joint hierarchy
    u32 JointsCount;
    mesh_animation* Animations; //Animation data
    u32 AnimationsCount;
};

struct mesh_animator
{
    mesh_joint* RootJoint;
    u32 JointsCount;
    mesh_animation* Animations;
    u32 AnimationsCount;
    float Time;
};
