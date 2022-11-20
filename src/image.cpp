#include "image.h"

internal u32
GetRGBAPixel(image_data* Image, s32 x, s32 y)
{
    Assert(Image->BytesPerPixel == 4);
    u32 Result = *(u32*)(Image->Data + Image->BytesPerPixel * x + Image->Pitch *  y);
    return Result;
}

internal image_data
LoadImageFromFile(char* Path, int BytesPerPixel = 4)
{
    stbi_set_flip_vertically_on_load(true);
    
    image_data Result = {};
    
    int ActualBytesPerPixel = 0;
    int* ActualBytesPerPixelPtr = 0;
    if(BytesPerPixel == 0) {
        ActualBytesPerPixelPtr = &ActualBytesPerPixel;
    }

    Result.Data = (u8*)stbi_load(Path, &Result.Width, &Result.Height, ActualBytesPerPixelPtr, BytesPerPixel);
    
    Result.BytesPerPixel = BytesPerPixel;
    if(BytesPerPixel == 0) {
        Result.BytesPerPixel = ActualBytesPerPixel;
    }
    
    Result.Pitch = Result.Width * Result.BytesPerPixel;
    
    return Result;
}

internal void
FreeImage(image_data* Image)
{
    stbi_image_free(Image->Data);
    *Image = {};
}

internal image_data
LoadHDRImageFromFile(char* Path)
{
    stbi_set_flip_vertically_on_load(false);
    
    image_data Result = {};
    s32 NumberOfChannels;
    Result.Data = (u8*)stbi_loadf(Path, &Result.Width, &Result.Height, &NumberOfChannels, 4);
    Assert(NumberOfChannels == 3);
    
    Result.BytesPerPixel = 4 * sizeof(f32);
    Result.Pitch = Result.BytesPerPixel * Result.Width;
    
    return Result;
}

internal image_data
CreateImage(void* Data, u32 Width, u32 Height, u32 Pitch, u32 BytesPerPixel)
{
    image_data ImageData = {};
    ImageData.Width = Width;
    ImageData.Height = Height;
    ImageData.Pitch = Pitch;
    ImageData.BytesPerPixel = BytesPerPixel;
    ImageData.Data = (u8*)Data;

    return ImageData;
}

internal image_3d_data
CreateImage3D(void* Data, s32 Width, s32 Height, s32 Depth, s32 BytesPerPixel)
{
    image_3d_data ImageData = {};
    ImageData.Width = Width;
    ImageData.Height = Height;
    ImageData.Depth = Depth;
    ImageData.BytesPerPixel = BytesPerPixel;
    ImageData.Data = (u8*)Data;
    
    return ImageData;
}

internal void
WriteImageToFile(image_data* Image, char* FileName)
{
    stbi_flip_vertically_on_write(true); // flag is non-zero to flip data vertically
    stbi_write_bmp(FileName, Image->Width, Image->Height, Image->BytesPerPixel, Image->Data);
}
