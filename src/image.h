#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

struct cubemap_data
{
    s32 Size;
    s32 Pitch;
    s32 BytesPerPixel;
    u32 NumberOfMips;
    
    u8* Data;
};

struct image_data
{
    s32 Width;
    s32 Height;
    s32 Pitch;
    s32 BytesPerPixel;
    
    u8* Data;
};

struct image_3d_data
{
    s32 Width;
    s32 Height;
    s32 Depth;
    s32 BytesPerPixel;
    
    u8* Data;
};