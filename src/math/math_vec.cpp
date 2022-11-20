//Types

struct ivec2;

struct vec2
{
    
    vec2() : x(0), y(0)
    {
    }
    
    
    explicit vec2(float _x, f32 _y) 
    {
        x = _x;
        y = _y;
    }
    
    explicit vec2(const ivec2&);
    
    explicit vec2(f32 v)
    {
        x = v;
        y = v;
    }
    
    inline vec2& operator+=(const vec2& L) 
    {
        x += L.x;
        y += L.y;
        return *this;
    }
    
    union
    {
        struct {f32 x, y;};
        struct {f32 r, g;};
        f32 e[2];
    };
};

struct vec4;
struct vec3
{
    vec3() : x(0), y(0), z(0)
    {
    }
    
    explicit vec3(const vec2& v, f32 _z) 
    {
        x = v.x;
        y = v.y;
        z = _z;
    }
    
    explicit vec3(f32 _x, f32 _y, f32 _z) 
    {
        x = _x;
        y = _y;
        z = _z;
    }
    
    explicit vec3(const vec4& v);
    
    explicit vec3(f32 v)
    {
        x = v;
        y = v;
        z = v;
    }
    
    inline vec3& operator+=(const vec3& L) 
    {
        x += L.x;
        y += L.y;
        z += L.z;
        return *this;
    }
    
    inline vec3& operator-=(const vec3& L) 
    {
        x -= L.x;
        y -= L.y;
        z -= L.z;
        return *this;
    }
    
    union
    {
        struct {f32 x, y, z;};
        struct {f32 r, g, b;};
        f32 e[3];
    };
};

struct vec4
{
    vec4() : x(0), y(0), z(0), w(0){
    }
    
    explicit vec4(f32 _x, f32 _y, f32 _z, f32 _w) 
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }
    
    explicit vec4(const vec2& v, f32 _z, f32 _w)
    {
        x = v.x;
        y = v.y;
        z = _z;
        w = _w;
    }
    
    explicit vec4(const vec3& v, f32 _w)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = _w;
    }
    
    explicit vec4(f32 v)
    {
        x = v;
        y = v;
        z = v;
        w = v;
    }
    
    union
    {
        struct {f32 x, y, z, w;};
        struct {f32 r, g, b, a;};
        f32 e[4];
        
#ifdef COMPILER_MSVC
        //Careful, this makes vec4 have a 16 byte align, 
        //can cause problems with struct packing
        //a struct containing a vec2 then a vec4 will occupy a full 32 bytes
        //while without this it would pack at 24, CAREFUL WHEN SERIALIZING STRUCTS
        __m128 v;
#endif
    };
};

vec3::vec3(const vec4& v)
{
    x = v.x;
    y = v.y;
    z = v.z;
}

struct ivec4
{
    union
    {
        struct {s32 x, y, z, w;};
        struct {s32 r, g, b, a;};
        s32 e[4];
    };
};

struct ivec2
{
    ivec2()
    {
        x = 0;
        y = 0;
    }
    
    explicit ivec2(s32 _x, s32 _y) 
    {
        x = _x;
        y = _y;
    }
    
    explicit ivec2(s32 _v) 
    {
        x = _v;
        y = _v;
    }
    
    explicit ivec2(const vec2& _v)
    {
        x = (s32)_v.x;
        y = (s32)_v.y;
    }
    
    union
    {
        struct {s32 x, y;};
        s32 e[2];
    };
};

vec2::vec2(const ivec2& v)
{
    x = (f32)v.x;
    y = (f32)v.y;
}

//OPERATORS
//ivec2
inline ivec2
operator+(ivec2 L, ivec2 R)
{
    return ivec2(L.x + R.x, L.y + R.y);
}

inline ivec2
operator+(s32 L, ivec2 R)
{
    return ivec2(L + R.x, L + R.y);
}

inline ivec2
operator+(ivec2 L, s32 R)
{
    return ivec2(L.x + R, L.y + R);
}

inline ivec2 
operator-(ivec2 L, ivec2 R)
{
    return ivec2(L.x - R.x, L.y - R.y);
}

inline ivec2
operator-(ivec2 L, s32 R)
{
    return ivec2(L.x - R, L.y - R);
}

inline ivec2
operator*(ivec2 L, s32 R)
{
    return ivec2(L.x * R, L.y * R);
}

inline ivec2
operator*(s32 L, ivec2 R)
{
    return ivec2(L * R.x, L * R.x);
}

inline ivec2
operator/(ivec2 L, s32 R)
{
    return ivec2(L.x / R, L.y / R);
}

inline bool
operator==(ivec2 L, ivec2 R)
{
    return L.x == R.x && L.y == R.y;
}

inline bool
operator!=(ivec2 L, ivec2 R)
{
    return L.x != R.x || L.y != R.y;
}

//vec2
inline vec2
operator+(vec2 L, vec2 R) 
{
    return vec2(L.x + R.x, L.y + R.y);
}

inline vec2
operator+(f32 L, vec2 R)
{
    return vec2(L + R.x, L + R.y);
}

inline vec2
operator+(vec2 L, f32 R)
{
    return vec2(L.x + R, L.y + R);
}

inline vec2 
operator-(vec2 L, vec2 R)
{
    return vec2(L.x - R.x, L.y - R.y);
}

inline vec2
operator-(vec2 L, f32 R)
{
    return vec2(L.x - R, L.y - R);
}

inline vec2
operator-(vec2 V)
{
    return vec2(-V.x, -V.y);
}

inline vec2
operator*(vec2 L, f32 R)
{
    return vec2(L.x * R, L.y * R);
}

inline vec2
operator*(f32 L, vec2 R)
{
    return vec2(L * R.x, L * R.y);
}

inline vec2
operator/(vec2 L, f32 R)
{
    return vec2(L.x / R, L.y / R);
}

inline bool
operator==(vec2 L, vec2 R)
{
    return L.x == R.x && L.y == R.y;
}

inline bool
operator!=(vec2 L, vec2 R)
{
    return L.x != R.x || L.y != R.y;
}

//vec3
inline vec3
operator+(vec3 L, vec3 R) 
{
    return vec3(L.x + R.x, L.y + R.y, L.z + R.z);
}

inline vec3
operator+(f32 L, vec3 R)
{
    return vec3(L + R.x, L + R.y, L + R.z);
}

inline vec3
operator+(vec3 L, f32 R)
{
    return vec3(L.x + R, L.y + R, L.z + R);
}

inline vec3 
operator-(vec3 L, vec3 R)
{
    return vec3(L.x - R.x, L.y - R.y, L.z - R.z);
}

inline vec3
operator-(vec3 L, f32 R)
{
    return vec3(L.x - R, L.y - R, L.z - R);
}

inline vec3
operator*(vec3 L, f32 R)
{
    return vec3(L.x * R, L.y * R, L.z * R);
}

inline vec3
operator*(f32 L, vec3 R)
{
    return vec3(L * R.x, L * R.y, L * R.z);
}

inline vec3
operator*(vec3 L, vec3 R)
{
    return vec3(L.x * R.x, L.y * R.y, L.z * R.z);
}

inline vec3
operator/(vec3 L, f32 R)
{
    return vec3(L.x / R, L.y / R, L.z / R);
}

inline vec3
operator/(f32 L, vec3 R)
{
    return vec3(L / R.x, L / R.y, L / R.z);
}

inline bool
operator==(vec3 L, vec3 R)
{
    return L.x == R.x && L.y == R.y && L.z == R.z;
}

inline bool
operator!=(vec3 L, vec3 R)
{
    return L.x != R.x || L.y != R.y || L.z != R.z;
}

inline vec3
operator-(vec3 V)
{
    return vec3(-V.x, -V.y, -V.z);
}

//vec4
inline vec4
operator+(vec4 L, vec4 R) 
{
    return vec4(L.x + R.x, L.y + R.y, L.z + R.z, L.w + R.w);
}

inline vec4
operator+(f32 L, vec4 R)
{
    return vec4(L + R.x, L + R.y, L + R.z, L + R.w);
}

inline vec4
operator+(vec4 L, f32 R)
{
    return vec4(L.x + R, L.y + R, L.z + R, L.w + R);
}

inline vec4 
operator-(vec4 L, vec4 R)
{
    return vec4(L.x - R.x, L.y - R.y, L.z - R.z, L.w - R.w);
}

inline vec4
operator-(vec4 L, f32 R)
{
    return vec4(L.x - R, L.y - R, L.z - R, L.w - R);
}

inline vec4
operator*(vec4 L, f32 R)
{
    return vec4(L.x * R, L.y * R, L.z * R, L.w * R);
}

inline vec4
operator*(f32 L, vec4 R)
{
    return vec4(L * R.x, L * R.y, L * R.z, L * R.w);
}

inline vec4
operator*(vec4 L, vec4 R)
{
    return vec4(L.x * R.x, L.y * R.y, L.z * R.z, L.w * R.w);
}

inline vec4
operator/(vec4 L, f32 R)
{
    return vec4(L.x / R, L.y / R, L.z / R, L.w / R);
}

inline bool
operator==(vec4 L, vec4 R)
{
    return L.x == R.x && L.y == R.y && L.z == R.z && L.w == R.w;
}

inline bool
operator!=(vec4 L, vec4 R)
{
    return L.x != R.x || L.y != R.y || L.z != R.z || L.w != R.w;
}

//Utils
inline vec2
Perpendicular(vec2 a)
{
    return vec2(-a.y, a.x);
}

inline f32
PerpDot(vec2 a, vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

inline f32
Dot(vec2 a, vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

inline f32
Dot(vec3 a, vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}


inline f32
Dot(vec4 a, vec4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

//Product of components
inline vec3
HadamardProduct(vec3 a, vec3 b)
{
    return vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline vec4
HadamardProduct(vec4 a, vec4 b)
{
    return vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

inline vec2
Clamp(vec2 v, vec2 min, vec2 max)
{
    vec2 Result = vec2(Clamp(v.x, min.x, max.x), 
                       Clamp(v.y, min.y, max.y));
    return Result;
}


inline vec3
Clamp(vec3 v, vec3 min, vec3 max)
{
    vec3 Result = vec3(Clamp(v.x, min.x, max.x), 
                       Clamp(v.y, min.y, max.y),
                       Clamp(v.z, min.z, max.z));
    return Result;
}

inline vec4
Clamp(vec4 v, vec4 min, vec4 max)
{
    vec4 Result = vec4(Clamp(v.x, min.x, max.x), 
                       Clamp(v.y, min.y, max.y),
                       Clamp(v.z, min.z, max.z),
                       Clamp(v.w, min.w, max.w));
    return Result;
}

inline vec3
Cross(vec3 a, vec3 b)
{
    vec3 Result;
    Result.x = a.y * b.z - a.z * b.y;
    Result.y = a.z * b.x - a.x * b.z;
    Result.z = a.x * b.y - a.y * b.x;
    
    return Result;
}

inline f32
LengthSquared(vec2 a)
{
    return Dot(a, a);
}

inline f32
Length(vec2 a)
{
    return (f32)sqrt(LengthSquared(a));
}

inline vec2
Normalize(vec2 a)
{
    return a / Length(a);
}

inline f32
LengthSquared(vec3 a)
{
    return Dot(a, a);
}

inline f32
Length(vec3 a)
{
    return (f32)sqrt(LengthSquared(a));
}

inline vec3
Normalize(vec3 a)
{
    return a / Length(a);
}

inline vec3
Negate(vec3 a)
{
    return a * -1.0f;
}

inline vec3
Lerp(vec3 a, vec3 b, float mix)
{
    vec3 Result;
    Result.x = Lerp(a.x, b.x, mix);
    Result.y = Lerp(a.y, b.y, mix);
    Result.z = Lerp(a.z, b.z, mix);
    
    return Result;
}

inline vec3
Bounce(vec3 v, vec3 n)
{
    return v - 2.0f * Dot(v, n) * n;
}

inline vec3
Refract(vec3 I, vec3 N, f32 RatioOfRefraction)
{
    f32 eta = RatioOfRefraction;
    
    f32 Cos = Dot(N, I);
    f32 k = 1.0f - eta * eta * (1.0f -  Cos * Cos);
    if (k < 0.0f)
        return vec3(0.0f);
    else
        return eta * I - (eta * Cos + sqrtf(k)) * N;
}

inline vec3
ExactLinearToSRGB(vec3 Color)
{
    vec3 Result;
    Result.x = ExactLinearToSRGB(Color.x);
    Result.y = ExactLinearToSRGB(Color.y);
    Result.z = ExactLinearToSRGB(Color.z);
    
    return Result;
}

inline vec4
ExactLinearToSRGB(vec4 Color)
{
    vec4 Result;
    Result.x = ExactLinearToSRGB(Color.x);
    Result.y = ExactLinearToSRGB(Color.y);
    Result.z = ExactLinearToSRGB(Color.z);
    Result.w = Color.w;
    
    return Result;
}

inline vec3
LinearToSRGB(vec3 Color)
{
    vec3 Result;
    Result.x = LinearToSRGB(Color.x);
    Result.y = LinearToSRGB(Color.y);
    Result.z = LinearToSRGB(Color.z);
    
    return Result;
}

inline vec4
LinearToSRGB(vec4 Color)
{
    vec4 Result;
    Result.x = LinearToSRGB(Color.x);
    Result.y = LinearToSRGB(Color.y);
    Result.z = LinearToSRGB(Color.z);
    Result.w = Color.w;
    
    return Result;
}

inline vec3
SRGBToLinear(vec3 Color)
{
    vec3 Result;
    Result.x = SRGBToLinear(Color.x);
    Result.y = SRGBToLinear(Color.y);
    Result.z = SRGBToLinear(Color.z);
    
    return Result;
}

inline vec4
SRGBToLinear(vec4 Color)
{
    vec4 Result;
    Result.x = SRGBToLinear(Color.x);
    Result.y = SRGBToLinear(Color.y);
    Result.z = SRGBToLinear(Color.z);
    Result.w = Color.w;
    
    return Result;
}

inline b32
IsNan(vec2 v)
{
    return isnan(v.x) || isnan(v.y);
}

inline b32
IsNan(vec3 v)
{
    return isnan(v.x) || isnan(v.y) || isnan(v.z);
}


inline u32
ClampVec4ToRGBA(vec4 v)
{
    v = Clamp(v, vec4(0.0f), vec4(1.0f));
    v = v * 255.0f + 0.5f; //Round instead of trunc
    u32 Result = RGBA((u8)v.r, (u8)v.g, (u8)v.b, (u8)v.a);
    return Result;
}

internal vec4
RGBAToVec4(u32 Color)
{
    u8 Red   = (u8)(Color >> 0);
    u8 Green = (u8)(Color >> 8);
    u8 Blue  = (u8)(Color >> 16);
    u8 Alpha = (u8)(Color >> 24);
    
    vec4 Result = vec4((f32)Red / 255, (f32)Green / 255, (f32)Blue / 255, (f32)Alpha / 255);
    return Result;
}


inline u32
ClampVec4ToBGRA(vec4 v)
{
    v = Clamp(v, vec4(0.0f), vec4(1.0f));
    v = (v * 255.0f) + 0.5f; //Round instead of trunc
    u32 Result = RGBA((u8)v.b, (u8)v.g, (u8)v.r, (u8)v.a);
    return Result;
}

internal vec4
BGRAToVec4(u32 Color)
{
    u8 Blue  = (u8)(Color >> 0);
    u8 Green = (u8)(Color >> 8);
    u8 Red   = (u8)(Color >> 16);
    u8 Alpha = (u8)(Color >> 24);
    
    vec4 Result = vec4((f32)Red / 255, (f32)Green / 255, (f32)Blue / 255, (f32)Alpha / 255);
    return Result;
}


inline vec3
HSVToRGB(f32 H, f32 S, f32 V)
{
    f32 C = S * V;
    f32 X = C * (1.0f - fabsf(fmodf(H / 60.0f, 2.0f) - 1.0f));
    
    vec3 Color;
    
    if(0 <= H && H < 60.0f)
        Color = vec3(C, X, 0);
    else if(60 <= H && H < 120.0f)
        Color = vec3(X, C, 0);
    else if(120 <= H && H < 180.0f)
        Color = vec3(0, C, X);
    else if(180 <= H && H < 240.0f)
        Color = vec3(0, X, C);
    else if(240 <= H && H < 360.0f)
        Color = vec3(X, 0, C);
    else
        Color = vec3(C, 0, X);
    
    f32 m = V - C;
    Color += vec3(m);
    return Color;
}


internal vec3
GetNormalCW(vec3 A, vec3 B, vec3 C)
{
    vec3 AB = B - A;
    vec3 AC = C - A;
    
    vec3 N = Normalize(Cross(AC, AB));
    return N;
}

internal vec3
GetNormalCCW(vec3 A, vec3 B, vec3 C)
{
    vec3 AB = B - A;
    vec3 AC = C - A;
    
    vec3 N = Normalize(Cross(AB, AC));
    return N;
}

internal vec3
Average(vec3* V, u32 Count)
{
    vec3 Result = vec3(0, 0, 0);
    for(u32 i = 0; i < Count; i++)
    {
        Result += V[i];
    }
    Result = Result * (1.0f / Count);
    return Result;
}


internal vec3
DirectionFromThetaPhi(float theta, float phi)
{
    float sintheta = sinf(theta);
    
    vec3 Result = vec3(sintheta * cosf(phi), sintheta * sinf(phi), cosf(theta));
    return Result;
}

internal vec3
DirectionFromThetaPhiDeg(float theta, float phi)
{
    return DirectionFromThetaPhi(DegToRad(theta), DegToRad(phi));
}

internal float
ThetaFromDirection(vec3 Dir)
{
    return acosf(Dir.z);
}

internal float
ThetaFromDirectionDeg(vec3 Dir)
{
    return RadToDeg(ThetaFromDirection(Dir));
}

internal float
PhiFromDirection(vec3 Dir)
{
    return atan2f(Dir.y, Dir.x);
}

internal float
PhiFromDirectionDeg(vec3 Dir)
{
    return RadToDeg(PhiFromDirection(Dir));
}


internal vec3
RandDir(random_series* Series)
{
    f32 Theta = RandRange(Series, -PI, PI);
    f32 Phi = RandRange(Series, 0, 2 * PI);
    
    return DirectionFromThetaPhi(Theta, Phi);
}

#ifdef ENABLE_GLOBAL_RAND_STATE
internal vec3
RandDir()
{
    return RandDir(&_RandSeries);
}
#endif
