#define PI 3.14159265359f


#ifdef COMPILER_MSVC
#include "intrin.h"
#endif

#if !defined(FLAME_MATH_LH) && !defined(FLAME_MATH_RH)
#define FLAME_MATH_RH
#endif

#if !defined(FLAME_MATH_DEPTH_ZO) && !defined(FLAME_MATH_DEPTH_NO)
#define FLAME_MATH_DEPTH_ZO
#endif

//Random utils
struct random_series
{
    u32 State;
};

internal random_series
RandSeries(u32 Seed)
{
    random_series Result;
    Result.State = Seed;
    
    return Result;
}

internal u32
XOrShift32(random_series *Series)
{
    // Reference XOrShift from https://en.wikipedia.org/wiki/Xorshift
    u32 x = Series->State;
    
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    
    Series->State = x;
    
    return(x);
}

internal u32
RandU32(random_series* Series)
{
    u32 Result = XOrShift32(Series);
    return Result;
}

inline f32
Randf(random_series* Series)
{
    return (float)(RandU32(Series) >> 1) / (UINT_MAX >> 1);
}

inline f32
RandRange(random_series* Series, f32 a, f32 b)
{
    return a + (b - a) * Randf(Series);
}

inline f32
RandNO(random_series* Series)
{
    return 2.0f * Randf(Series) - 1.0f;
}

#ifdef ENABLE_GLOBAL_RAND_STATE
// Random functions using the global state
random_series _RandSeries = {22389476};

internal void
RandSeed(u32 Seed)
{
    _RandSeries.State = Seed;
}

internal u32
RandU32()
{
    return RandU32(&_RandSeries);
}

inline f32
Randf()
{
    return Randf(&_RandSeries);
}

inline f32
RandRange(f32 a, f32 b)
{
    return RandRange(&_RandSeries, a, b);
}

inline f32
RandNO()
{
    return RandNO(&_RandSeries);
}
#endif


//Convertion utils
inline f32
DegToRad(f32 Deg)
{
    f32 Rad = Deg * (PI / 180.0f);
    return Rad;
}

inline f32
RadToDeg(f32 Rad)
{
    f32 Deg = Rad * (180.0f / PI);
    return Deg;
}


//Clamp
inline s32
ClampS32(s32 Val, s32 Min, s32 Max)
{
    if(Min > Max)
    {
        return Max;
    }
    
    if(Val < Min)
    {
        return Min;
    }
    else if(Val > Max)
    {
        return Max;
    }
    else
    {
        return Val;
    }
}

inline f32
Clamp(f32 Val, f32 Min, f32 Max)
{
    if(Min > Max)
    {
        return Max;
    }
    
    if(Val < Min)
    {
        return Min;
    }
    else if(Val > Max)
    {
        return Max;
    }
    else
    {
        return Val;
    }
}


//Float lerp
internal f32
Lerp(f32 a, f32 b, f32 mix)
{
    return a * (1.0f - mix) + b * mix;
}


//RGB
inline u32
RGBA(u8 R, u8 G, u8 B, u8 A)
{
    u32 Result = 0;
    Result |= R;
    Result |= G << 8;
    Result |= B << 16;
    Result |= A << 24;
    
    return Result;
}

#undef RGB
inline u32
RGB(u8 Red, u8 Green, u8 Blue)
{
    return RGBA(Red, Green, Blue, 255);
}

inline u32
RGBToBGR(u32 Pixel)
{
    u32 R = Pixel & 0xFF;
    u32 B = (Pixel >> 16) & 0xFF;
    
    return (Pixel & 0xFF00FF00) | (B) | (R << 16);
}

inline f32
ExactLinearToSRGB(f32 v)
{
    v = Clamp(v, 0.0f, 1.0f);
    if(v > 0.0031308f)
    {
        return 1.055f * (f32)pow(v, 1.0f/2.4f) - 0.055f;
    } 
    else 
    {
        return v * 12.92f;
    }
}

inline f32
LinearToSRGB(f32 v)
{
    return (f32)pow(v, 1.0/2.2);
}

inline f32
SRGBToLinear(f32 v)
{
    return (f32)pow(v, 2.2);
}
