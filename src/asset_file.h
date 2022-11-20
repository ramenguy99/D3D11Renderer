#pragma pack(push, 1)

#define ASSET_FILE_MAGIC 0xAF00AF00
#define ASSET_NAME_LENGTH 80
#define ASSET_TAG_LENGTH 28

struct asset_file_header
{
    u32 Magic;
    u32 Version;
    u64 TableEntryCount; //Size of table is TableEntryCount * sizeof(asset_table_entry)
    //Table begins right after header
};

enum asset_type
{
    ASSET_NONE,
    ASSET_MESH,
    ASSET_IMAGE,
    ASSET_SOUND,
    ASSET_CUBEMAP,
};

char* AssetTypeToName[] = 
{
    "ASSET_NONE",
    "ASSET_MESH",
    "ASSET_IMAGE",
    "ASSET_SOUND",
    "ASSET_CUBEMAP",
};

struct asset_table_entry
{
    char Name[ASSET_NAME_LENGTH];
    char Tag[ASSET_TAG_LENGTH];
    asset_type Type;
    u64 Offset;
    u64 Size;
};

struct asset_table
{
    asset_table_entry* Entries;
    u64 Count;
};

struct asset_image
{
    u32 Width;
    u32 Height;
    u32 BytesPerPixel;
    u32 Pitch;
    //Data begins right after
};

struct asset_mesh
{
    u32 Flags;
    u32 VerticesCount;
    u32 IndicesCount;
    u32 TexturesCount;
    u32 JointsCount;
    u32 AnimationsCount;
    
    //Vertices are stored as contiguos arrays in this order
    //Positions - Normals - Tangents - UVs - Weights - Joints - Indices
    //weights and joints are there only if the MESH_HAS_ANIMATION flag is set
    u32 VertexDataOffset;
    
    //Textures are stored as an array TexturesCount names of ASSET_NAME_LENGTH chars
    //that refer to assets in the file
    u32 TextureAssetsOffset;
    
    //Those are 0s if the MESH_HAS_ANIMATION flag is NOT set
    u32 JointsOffset;
    u32 AnimationsOffset;
};

struct asset_cubemap
{
    s32 Size; //Width and height of a face
    s32 Pitch; //Size * BytesPerPixel
    s32 BytesPerPixel;
    u32 NumberOfMips; //0 is the same as 1, basically just the image
    //Data starts here with 6 faces of Size dimension,
    //If has 1st level mips then 6 face of Size / 2 dimension, etc..
};


#pragma pack(pop)
