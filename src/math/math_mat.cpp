//Column major as d3d and opengl expect those for (M * v) multiplies
struct mat4
{
    mat4()
    {
        Columns[0] = vec4(0.0f);
        Columns[1] = vec4(0.0f);
        Columns[2] = vec4(0.0f);
        Columns[3] = vec4(0.0f);
    }
    
    mat4(f32 v)
    {
        Columns[0] = vec4(0.0f);
        Columns[1] = vec4(0.0f);
        Columns[2] = vec4(0.0f);
        Columns[3] = vec4(0.0f);
        
        e[0][0] = v;
        e[1][1] = v;
        e[2][2] = v;
        e[3][3] = v;
    }
    
    union
    {
        f32 e[4][4];
        vec4 Columns[4];
    };
};

struct mat3
{
    mat3() : Columns{vec3(0.0f), vec3(0.0f), vec3(0.0f)}
    {
    }
    
    explicit mat3(f32 v)
    {
        Columns[0] = vec3(0.0f);
        Columns[1] = vec3(0.0f);
        Columns[2] = vec3(0.0f);
        
        e[0][0] = v;
        e[1][1] = v;
        e[2][2] = v;
    }
    
    union
    {
        f32 e[3][3];
        vec3 Columns[3];
    };
};

inline mat3
Mat3SkewSymmetric(vec3 v)
{
    mat3 Result(0.0f);
    
    Result.e[0][1] = v.z;
    Result.e[0][2] = -v.y;
    
    Result.e[1][0] = -v.z;
    Result.e[1][2] = v.x;
    
    Result.e[2][0] = v.y;
    Result.e[2][1] = -v.x;
    
    return Result;
}

inline mat3
Mat3Identity()
{
    mat3 Result(1.0f);
    return Result;
}

inline mat3
Mat3Scale(vec3 v)
{
    mat3 Result(0.0f);
    Result.e[0][0] = v.x;
    Result.e[1][1] = v.y;
    Result.e[2][2] = v.z;
    
    return Result;
}

inline mat3
Mat3Rotate(vec3 Axis, f32 AngleDegrees)
{
    mat3 Result(0.0f);
    
    f32 t = DegToRad(AngleDegrees);
    
    f32 cos_t = cosf(t);
    f32 sin_t = sinf(t);
    f32 inv_cos = 1 - cos_t;
    
    f32 x = Axis.x;
    f32 y = Axis.y;
    f32 z = Axis.z;
    
    Result.e[0][0] = cos_t + x * x* inv_cos;
    Result.e[0][1] = y * x * inv_cos + z * sin_t;
    Result.e[0][2] = z * x * inv_cos - y * sin_t;
    
    Result.e[1][0] = x * y * inv_cos - z * sin_t;
    Result.e[1][1] = cos_t + y * y* inv_cos;
    Result.e[1][2] = z * y * inv_cos + x * sin_t;
    
    Result.e[2][0] = x * z * inv_cos + y * sin_t;
    Result.e[2][1] = y * z * inv_cos - x * sin_t;
    Result.e[2][2] = cos_t + z * z * inv_cos;
    
    return Result;
}


inline void
Mat3Orthonormalize(mat3* V)
{
    vec3 x = V->Columns[0];
    vec3 y = V->Columns[1];
    vec3 z = V->Columns[2];
    
    x = Normalize(x);
    z = Normalize(Cross(x, y));
    y = Normalize(Cross(z, x));
    
    V->Columns[0] = x;
    V->Columns[1] = y;
    V->Columns[2] = z;
}

inline mat3
Mat3Transpose(mat3& L)
{
    vec3 Rows[] = {
        vec3(L.e[0][0], L.e[1][0], L.e[2][0]),
        vec3(L.e[0][1], L.e[1][1], L.e[2][1]),
        vec3(L.e[0][2], L.e[1][2], L.e[2][2]),
    };
    
    mat3 Result(0.0f);
    Result.Columns[0] = Rows[0];
    Result.Columns[1] = Rows[1];
    Result.Columns[2] = Rows[2];
    
    return Result;
}

inline mat3
operator+(const mat3& L, const mat3& R)
{
    mat3 Result(0.0f);
    
    //i column
    for(u32 i = 0; i < 3; i++)
    {
        Result.Columns[i] = L.Columns[i] + R.Columns[i];
    }
    
    return Result;
}

inline mat3
operator*(const mat3& L, const mat3& R)
{
    mat3 Result(0.0f);
    
    vec3 Rows[] = {
        vec3(L.e[0][0], L.e[1][0], L.e[2][0]),
        vec3(L.e[0][1], L.e[1][1], L.e[2][1]),
        vec3(L.e[0][2], L.e[1][2], L.e[2][2]),
    };
    
    //i column
    for(u32 i = 0; i < 3; i++)
    {
        //j row
        for(u32 j = 0; j < 3; j++)
        {
            Result.e[i][j] = Dot(Rows[j], R.Columns[i]);
        }
    }
    
    return Result;
}

inline vec3
operator*(const mat3& L, const vec3& R)
{
    vec3 Result;
    
    vec3 Rows[] = {
        vec3(L.e[0][0], L.e[1][0], L.e[2][0]),
        vec3(L.e[0][1], L.e[1][1], L.e[2][1]),
        vec3(L.e[0][2], L.e[1][2], L.e[2][2]),
    };
    
    //i column
    for(u32 i = 0; i < 3; i++)
    {
        Result.e[i] = Dot(Rows[i], R);
    }
    
    return Result;
}


inline mat3
Mat3FromEulerXYZ(vec3 EulerXYZ) //Euler angles in degrees for rotation around X, Y, Z axis
{
    f32 cosx = cosf(DegToRad(EulerXYZ.x));
    f32 sinx = sinf(DegToRad(EulerXYZ.x));
    
    mat3 Rx = mat3(1.0f);
    Rx.e[1][1] = cosx;
    Rx.e[1][2] = sinx;
    Rx.e[2][1] = -sinx;
    Rx.e[2][2] = cosx;
    
    f32 cosy = cosf(DegToRad(EulerXYZ.y));
    f32 siny = sinf(DegToRad(EulerXYZ.y));
    
    mat3 Ry = mat3(1.0f);
    Ry.e[0][0] = cosy;
    Ry.e[0][2] = -siny;
    Ry.e[2][0] = siny;
    Ry.e[2][2] = cosy;
    
    f32 cosz = cosf(DegToRad(EulerXYZ.z));
    f32 sinz = sinf(DegToRad(EulerXYZ.z));
    
    mat3 Rz = mat3(1.0f);
    Rz.e[0][0] = cosz;
    Rz.e[0][1] = sinz;
    Rz.e[1][0] = -sinz;
    Rz.e[1][1] = cosz;
    
    
    return Rz * Ry * Rx;
}


/*
inline mat4
Mat4Orthographic(float32 Left, float32 Right, float32 Top, float32 Bottom)
{
    mat4 Result = Mat4Identity();
    Result.e[0][0] =  2.0f / (Right - Left);
    Result.e[1][1] =  2.0f / (Top - Bottom);
    Result.e[3][0] = - (Right + Left) / (Right - Left);
    Result.e[3][1] = - (Top + Bottom) / (Top - Bottom);
    
    return Result;
}
*/



inline mat4
Mat4Identity()
{
    mat4 Result(1.0f);
    return Result;
}

inline mat4
Mat4OrthographicLH(float32 Left, float32 Right, float32 Bottom, float32 Top, float32 Near, float32 Far)
{
    mat4 Result = Mat4Identity();
    
#if defined(FLAME_MATH_DEPTH_ZO)
    Result.e[0][0] = 2.0f / (Right - Left);
    Result.e[1][1] = 2.0f / (Top - Bottom);
    Result.e[2][2] = 1.0f / (Far - Near);
    Result.e[3][0] = - (Right + Left) / (Right - Left);
    Result.e[3][1] = - (Top + Bottom) / (Top - Bottom);
    Result.e[3][2] = - Near / (Far - Near);
    
#elif defined(FLAME_MATH_DEPTH_NO)
    Result.e[0][0] = 2.0f / (Right - Left);
    Result.e[1][1] = 2.0f / (Top - Bottom);
    Result.e[2][2] = 2.0f / (Far - Near);
    Result.e[3][0] = - (Right + Left) / (Right - Left);
    Result.e[3][1] = - (Top + Bottom) / (Top - Bottom);
    Result.e[3][2] = - (Far + Near) / (Far - Near);
#else
#error missing depth defines
#endif
    
    return Result;
}


inline mat4
Mat4OrthographicRH(float32 Left, float32 Right, float32 Bottom, float32 Top, float32 Near, float32 Far)
{
    mat4 Result = Mat4Identity();
    
#if defined(FLAME_MATH_DEPTH_ZO)
    Result.e[0][0] = 2.0f / (Right - Left);
    Result.e[1][1] = 2.0f / (Top - Bottom);
    Result.e[2][2] = - 1.0f / (Far - Near);
    Result.e[3][0] = - (Right + Left) / (Right - Left);
    Result.e[3][1] = - (Top + Bottom) / (Top - Bottom);
    Result.e[3][2] = - Near / (Far - Near);
    
#elif defined(FLAME_MATH_DEPTH_NO)
    Result.e[0][0] = 2.0f / (Right - Left);
    Result.e[1][1] = 2.0f / (Top - Bottom);
    Result.e[2][2] = - 2.0f / (Far - Near); //2.0f for opengl
    Result.e[3][0] = - (Right + Left) / (Right - Left);
    Result.e[3][1] = - (Top + Bottom) / (Top - Bottom);
    Result.e[3][2] = - (Far + Near) / (Far - Near);
#else
#error missing depth defines
#endif
    
    return Result;
}

inline mat4
Mat4Orthographic(float32 Left, float32 Right, float32 Bottom, float32 Top, float32 Near, float32 Far)
{
    mat4 Result = Mat4Identity();
    
#if   defined(FLAME_MATH_LH)
    return Mat4OrthographicLH(Left, Right, Bottom, Top, Near, Far);
#elif defined(FLAME_MATH_RH)
    return Mat4OrthographicRH(Left, Right, Bottom, Top, Near, Far);
#else
#error missing handedness defines
#endif
}

inline mat4
Mat4FromMat3AndTranslation(mat3 M, vec3 Translation)
{
    mat4 Result;
    Result.Columns[0] = vec4(M.Columns[0], 0.0f);
    Result.Columns[1] = vec4(M.Columns[1], 0.0f);
    Result.Columns[2] = vec4(M.Columns[2], 0.0f);
    Result.Columns[3] = vec4(Translation, 1.0f);
    
    return Result;
}

inline mat4
Mat4Translation(vec3 Translation)
{
    mat4 Result = Mat4Identity();
    Result.Columns[3].x = Translation.x;
    Result.Columns[3].y = Translation.y;
    Result.Columns[3].z = Translation.z;
    
    return Result;
}

inline mat4
Mat4Translation(float x, float y, float z)
{
    mat4 Result = Mat4Identity();
    Result.Columns[3].x = x;
    Result.Columns[3].y = y;
    Result.Columns[3].z = z;
    
    return Result;
}

inline mat4
Mat4PerspectiveLH(f32 FOVDegrees,  f32 Near, f32 Far, f32 AspectRatio = 16.0f / 9.0f)
{
    mat4 Result = {};
    
    f32 alpha = DegToRad(FOVDegrees);
    f32 t = (f32)tan(alpha / 2.0f);
    
#if defined(FLAME_MATH_DEPTH_ZO)
    Result.e[0][0] = 1.0f / (AspectRatio * t);
    Result.e[1][1] = 1.0f / t;
    Result.e[2][2] = Far / (Far - Near);
    Result.e[2][3] = 1.0f;
    Result.e[3][2] = -(Near * Far) / (Far - Near);
    
#elif defined(FLAME_MATH_DEPTH_NO)
    Result.e[0][0] = 1.0f / (AspectRatio * t);
    Result.e[1][1] = 1.0f / t;
    Result.e[2][2] = (Far + Near) / (Far - Near);
    Result.e[2][3] = 1.0f;
    Result.e[3][2] = -(Far * Near) / (Far - Near);
#else
#error missing depth defines
#endif
    
    return Result;
}


inline mat4
Mat4PerspectiveRH(f32 FOVDegrees,  f32 Near, f32 Far, f32 AspectRatio = 16.0f / 9.0f)
{
    mat4 Result = {};
    
    f32 alpha = DegToRad(FOVDegrees);
    f32 t = (f32)tan(alpha / 2.0f);
#if defined(FLAME_MATH_DEPTH_ZO)
    Result.e[0][0] = 1.0f / (AspectRatio * t);
    Result.e[1][1] = 1.0f / t;
    Result.e[2][2] = Far / (Near - Far);
    Result.e[2][3] = - 1.0f;
    Result.e[3][2] = -(Near * Far) / (Far - Near);
    
#elif defined(FLAME_MATH_DEPTH_NO)
    Result.e[0][0] = 1.0f / (AspectRatio * t);
    Result.e[1][1] = 1.0f / t;
    Result.e[2][2] = - (Far + Near) / (Far - Near);
    Result.e[2][3] = - 1.0f;
    Result.e[3][2] = - (2.0f * Far * Near) / (Far - Near);
#else
#error missing depth defines
#endif
    
    return Result;
}

inline mat4
Mat4Perspective(f32 FOVDegrees,  f32 Near, f32 Far, f32 AspectRatio = 16.0f / 9.0f)
{    
#if defined(FLAME_MATH_LH)
    return Mat4PerspectiveLH(FOVDegrees, Near, Far, AspectRatio);
#elif defined(FLAME_MATH_RH)
    return Mat4PerspectiveRH(FOVDegrees, Near, Far, AspectRatio);
#else
#error missing handedness defines
#endif
}

inline mat4
Mat4LinearZPerspectiveLH(f32 FOVDegrees,  f32 Near, f32 Far, f32 AspectRatio = 16.0f / 9.0f)
{
    mat4 Result = {};
    
    f32 alpha = DegToRad(FOVDegrees);
    f32 t = (f32)tan(alpha / 2.0f);
    
#if defined(FLAME_MATH_DEPTH_ZO)
    Result.e[0][0] = 1.0f / (AspectRatio * t);
    Result.e[1][1] = 1.0f / t;
    Result.e[2][2] = 1 / (Far - Near);
    Result.e[2][3] = 1.0f;
    Result.e[3][2] = Near / (Far - Near);
#else
#error todo: linear z for depth NO
#endif
    
    return Result;
}

inline mat4
Mat4LinearZPerspectiveRH(f32 FOVDegrees,  f32 Near, f32 Far, f32 AspectRatio = 16.0f / 9.0f)
{
    mat4 Result = {};
    
    f32 alpha = DegToRad(FOVDegrees);
    f32 t = (f32)tan(alpha / 2.0f);
    
#if defined(FLAME_MATH_DEPTH_ZO)
    Result.e[0][0] = 1.0f / (AspectRatio * t);
    Result.e[1][1] = 1.0f / t;
    Result.e[2][2] = - 1 / (Far - Near);
    Result.e[2][3] = - 1.0f;
    Result.e[3][2] = - Near / (Far - Near);
#else
#error todo: linear z for depth NO
#endif
    
    return Result;
}

inline mat4
Mat4LinearZPerspective(f32 FOVDegrees,  f32 Near, f32 Far, f32 AspectRatio = 16.0f / 9.0f)
{
#if defined(FLAME_MATH_LH)
    return Mat4LinearZPerspectiveLH(FOVDegrees, Near, Far, AspectRatio);
#elif defined(FLAME_MATH_RH)
    return Mat4LinearZPerspectiveRH(FOVDegrees, Near, Far, AspectRatio);
#else
#error missing handedness defines
#endif
}

inline mat4
Mat4Rotate(f32 AngleDegrees, vec3 Axis)
{
    mat4 Result(0.0f);
    f32 t = DegToRad(AngleDegrees);
    
    f32 cos_t = (f32)cos(t);
    f32 sin_t = (f32)sin(t);
    f32 inv_cos = 1 - cos_t;
    
    f32 x = Axis.x;
    f32 y = Axis.y;
    f32 z = Axis.z;
    
    Result.e[0][0] = cos_t + x * x* inv_cos;
    Result.e[0][1] = y * x * inv_cos + z * sin_t;
    Result.e[0][2] = z * x * inv_cos - y * sin_t;
    Result.e[0][3] = 0;
    
    Result.e[1][0] = x * y * inv_cos - z * sin_t;
    Result.e[1][1] = cos_t + y * y* inv_cos;
    Result.e[1][2] = z * y * inv_cos + x * sin_t;
    Result.e[1][3] = 0;
    
    Result.e[2][0] = x * z * inv_cos + y * sin_t;
    Result.e[2][1] = y * z * inv_cos - x * sin_t;
    Result.e[2][2] = cos_t + z * z * inv_cos;
    Result.e[2][3] = 0;
    
    Result.Columns[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    return Result;
}

inline mat4
Mat4Scale(f32 x, f32 y, f32 z)
{
    mat4 Result(0.0f);
    
    Result.e[0][0] = x;
    Result.e[1][1] = y;
    Result.e[2][2] = z;
    Result.e[3][3] = 1.0f;
    
    return Result;
}

inline mat4
Mat4Scale(vec3 ScaleVec)
{
    return Mat4Scale(ScaleVec.x, ScaleVec.y, ScaleVec.z);
}

inline mat4
Mat4LookAtLH(vec3 P, vec3 At, vec3 Up = vec3(0.0f, 1.0f, 0.0f))
{
    vec3 f = Normalize(At - P);
    vec3 r = Normalize(Cross(Up, f));
    vec3 u = Cross(f, r);
    
    mat4 Result(0.0f);
    Result.e[0][0] = r.x;
    Result.e[1][0] = r.y;
    Result.e[2][0] = r.z;
    
    Result.e[0][1] = u.x;
    Result.e[1][1] = u.y;
    Result.e[2][1] = u.z;
    
    Result.e[0][2] = f.x;
    Result.e[1][2] = f.y;
    Result.e[2][2] = f.z;
    
    Result.e[0][3] = 0;
    Result.e[1][3] = 0;
    Result.e[2][3] = 0;
    
    Result.e[3][0] = -Dot(r, P);
    Result.e[3][1] = -Dot(u, P);
    Result.e[3][2] = -Dot(f, P);
    Result.e[3][3] = 1.0f;
    
    return Result;
}

inline mat4
Mat4LookAtRH(vec3 P, vec3 At, vec3 Up = vec3(0.0f, 1.0f, 0.0f))
{
    vec3 f = Normalize(At - P);
    vec3 r = Normalize(Cross(f, Up));
    vec3 u = Cross(r, f);
    
    mat4 Result(0.0f);
    Result.e[0][0] = r.x;
    Result.e[1][0] = r.y;
    Result.e[2][0] = r.z;
    
    Result.e[0][1] = u.x;
    Result.e[1][1] = u.y;
    Result.e[2][1] = u.z;
    
    Result.e[0][2] = -f.x;
    Result.e[1][2] = -f.y;
    Result.e[2][2] = -f.z;
    
    Result.e[0][3] = 0;
    Result.e[1][3] = 0;
    Result.e[2][3] = 0;
    
    Result.e[3][0] = -Dot(r, P);
    Result.e[3][1] = -Dot(u, P);
    Result.e[3][2] = Dot(f, P);
    Result.e[3][3] = 1.0f;
    
    return Result;
}

inline mat4
Mat4LookAt(vec3 P, vec3 At, vec3 Up = vec3(0.0f, 1.0f, 0.0f))
{
#if defined(FLAME_MATH_LH)
    return Mat4LookAtLH(P, At, Up);
#elif defined(FLAME_MATH_RH)
    return Mat4LookAtRH(P, At, Up);
#else
#error missing handedness defines
#endif
}

inline mat4
operator*(const mat4& L, const mat4& R)
{
    mat4 Result;
    
    vec4 Rows[] = {
        vec4(L.e[0][0], L.e[1][0], L.e[2][0], L.e[3][0]),
        vec4(L.e[0][1], L.e[1][1], L.e[2][1], L.e[3][1]),
        vec4(L.e[0][2], L.e[1][2], L.e[2][2], L.e[3][2]),
        vec4(L.e[0][3], L.e[1][3], L.e[2][3], L.e[3][3]),
    };
    
    //i column
    for(u32 i = 0; i < 4; i++)
    {
        //j row
        for(u32 j = 0; j < 4; j++)
        {
            Result.e[i][j] = Dot(Rows[j], R.Columns[i]);
        }
    }
    
    return Result;
}

inline vec4
operator*(const mat4& L, const vec4& R)
{
    vec4 Result;
    
    vec4 Rows[] = {
        vec4(L.e[0][0], L.e[1][0], L.e[2][0], L.e[3][0]),
        vec4(L.e[0][1], L.e[1][1], L.e[2][1], L.e[3][1]),
        vec4(L.e[0][2], L.e[1][2], L.e[2][2], L.e[3][2]),
        vec4(L.e[0][3], L.e[1][3], L.e[2][3], L.e[3][3]),
    };
    
    //i column
    for(u32 i = 0; i < 4; i++)
    {
        Result.e[i] = Dot(Rows[i], R);
    }
    
    return Result;
}

internal mat4
Mat4Transpose(mat4 Input)
{
    mat4 Result;
    vec4 Rows[] = {
        vec4(Input.e[0][0], Input.e[1][0], Input.e[2][0], Input.e[3][0]),
        vec4(Input.e[0][1], Input.e[1][1], Input.e[2][1], Input.e[3][1]),
        vec4(Input.e[0][2], Input.e[1][2], Input.e[2][2], Input.e[3][2]),
        vec4(Input.e[0][3], Input.e[1][3], Input.e[2][3], Input.e[3][3]),
    };
    
    Result.Columns[0] = Rows[0];
    Result.Columns[1] = Rows[1];
    Result.Columns[2] = Rows[2];
    Result.Columns[3] = Rows[3];
    
    return Result;
}

#ifdef COMPILER_MSVC

internal mat4
Mat4Inverse(mat4 Input)
{
    __m128 in[4] = {
        Input.Columns[0].v,
        Input.Columns[1].v,
        Input.Columns[2].v,
        Input.Columns[3].v
    };
    
	__m128 Fac0;
	{
		//	valType SubFactor00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		//	valType SubFactor00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		//	valType SubFactor06 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		//	valType SubFactor13 = m[1][2] * m[2][3] - m[2][2] * m[1][3];
        
		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));
        
		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));
        
		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac0 = _mm_sub_ps(Mul00, Mul01);
	}
    
	__m128 Fac1;
	{
		//	valType SubFactor01 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		//	valType SubFactor01 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		//	valType SubFactor07 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		//	valType SubFactor14 = m[1][1] * m[2][3] - m[2][1] * m[1][3];
        
		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));
        
		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));
        
		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac1 = _mm_sub_ps(Mul00, Mul01);
	}
    
    
	__m128 Fac2;
	{
		//	valType SubFactor02 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		//	valType SubFactor02 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		//	valType SubFactor08 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
		//	valType SubFactor15 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
        
		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));
        
		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));
        
		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac2 = _mm_sub_ps(Mul00, Mul01);
	}
    
	__m128 Fac3;
	{
		//	valType SubFactor03 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		//	valType SubFactor03 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		//	valType SubFactor09 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		//	valType SubFactor16 = m[1][0] * m[2][3] - m[2][0] * m[1][3];
        
		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(3, 3, 3, 3));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));
        
		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(3, 3, 3, 3));
        
		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac3 = _mm_sub_ps(Mul00, Mul01);
	}
    
	__m128 Fac4;
	{
		//	valType SubFactor04 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		//	valType SubFactor04 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		//	valType SubFactor10 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		//	valType SubFactor17 = m[1][0] * m[2][2] - m[2][0] * m[1][2];
        
		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(2, 2, 2, 2));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));
        
		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(2, 2, 2, 2));
        
		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac4 = _mm_sub_ps(Mul00, Mul01);
	}
    
	__m128 Fac5;
	{
		//	valType SubFactor05 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		//	valType SubFactor05 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		//	valType SubFactor12 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		//	valType SubFactor18 = m[1][0] * m[2][1] - m[2][0] * m[1][1];
        
		__m128 Swp0a = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(1, 1, 1, 1));
		__m128 Swp0b = _mm_shuffle_ps(in[3], in[2], _MM_SHUFFLE(0, 0, 0, 0));
        
		__m128 Swp00 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(0, 0, 0, 0));
		__m128 Swp01 = _mm_shuffle_ps(Swp0a, Swp0a, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp02 = _mm_shuffle_ps(Swp0b, Swp0b, _MM_SHUFFLE(2, 0, 0, 0));
		__m128 Swp03 = _mm_shuffle_ps(in[2], in[1], _MM_SHUFFLE(1, 1, 1, 1));
        
		__m128 Mul00 = _mm_mul_ps(Swp00, Swp01);
		__m128 Mul01 = _mm_mul_ps(Swp02, Swp03);
		Fac5 = _mm_sub_ps(Mul00, Mul01);
	}
    
	__m128 SignA = _mm_set_ps( 1.0f,-1.0f, 1.0f,-1.0f);
	__m128 SignB = _mm_set_ps(-1.0f, 1.0f,-1.0f, 1.0f);
    
	// m[1][0]
	// m[0][0]
	// m[0][0]
	// m[0][0]
	__m128 Temp0 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Vec0 = _mm_shuffle_ps(Temp0, Temp0, _MM_SHUFFLE(2, 2, 2, 0));
    
	// m[1][1]
	// m[0][1]
	// m[0][1]
	// m[0][1]
	__m128 Temp1 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(1, 1, 1, 1));
	__m128 Vec1 = _mm_shuffle_ps(Temp1, Temp1, _MM_SHUFFLE(2, 2, 2, 0));
    
	// m[1][2]
	// m[0][2]
	// m[0][2]
	// m[0][2]
	__m128 Temp2 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(2, 2, 2, 2));
	__m128 Vec2 = _mm_shuffle_ps(Temp2, Temp2, _MM_SHUFFLE(2, 2, 2, 0));
    
	// m[1][3]
	// m[0][3]
	// m[0][3]
	// m[0][3]
	__m128 Temp3 = _mm_shuffle_ps(in[1], in[0], _MM_SHUFFLE(3, 3, 3, 3));
	__m128 Vec3 = _mm_shuffle_ps(Temp3, Temp3, _MM_SHUFFLE(2, 2, 2, 0));
    
	// col0
	// + (Vec1[0] * Fac0[0] - Vec2[0] * Fac1[0] + Vec3[0] * Fac2[0]),
	// - (Vec1[1] * Fac0[1] - Vec2[1] * Fac1[1] + Vec3[1] * Fac2[1]),
	// + (Vec1[2] * Fac0[2] - Vec2[2] * Fac1[2] + Vec3[2] * Fac2[2]),
	// - (Vec1[3] * Fac0[3] - Vec2[3] * Fac1[3] + Vec3[3] * Fac2[3]),
	__m128 Mul00 = _mm_mul_ps(Vec1, Fac0);
	__m128 Mul01 = _mm_mul_ps(Vec2, Fac1);
	__m128 Mul02 = _mm_mul_ps(Vec3, Fac2);
	__m128 Sub00 = _mm_sub_ps(Mul00, Mul01);
	__m128 Add00 = _mm_add_ps(Sub00, Mul02);
	__m128 Inv0 = _mm_mul_ps(SignB, Add00);
    
	// col1
	// - (Vec0[0] * Fac0[0] - Vec2[0] * Fac3[0] + Vec3[0] * Fac4[0]),
	// + (Vec0[0] * Fac0[1] - Vec2[1] * Fac3[1] + Vec3[1] * Fac4[1]),
	// - (Vec0[0] * Fac0[2] - Vec2[2] * Fac3[2] + Vec3[2] * Fac4[2]),
	// + (Vec0[0] * Fac0[3] - Vec2[3] * Fac3[3] + Vec3[3] * Fac4[3]),
	__m128 Mul03 = _mm_mul_ps(Vec0, Fac0);
	__m128 Mul04 = _mm_mul_ps(Vec2, Fac3);
	__m128 Mul05 = _mm_mul_ps(Vec3, Fac4);
	__m128 Sub01 = _mm_sub_ps(Mul03, Mul04);
	__m128 Add01 = _mm_add_ps(Sub01, Mul05);
	__m128 Inv1 = _mm_mul_ps(SignA, Add01);
    
	// col2
	// + (Vec0[0] * Fac1[0] - Vec1[0] * Fac3[0] + Vec3[0] * Fac5[0]),
	// - (Vec0[0] * Fac1[1] - Vec1[1] * Fac3[1] + Vec3[1] * Fac5[1]),
	// + (Vec0[0] * Fac1[2] - Vec1[2] * Fac3[2] + Vec3[2] * Fac5[2]),
	// - (Vec0[0] * Fac1[3] - Vec1[3] * Fac3[3] + Vec3[3] * Fac5[3]),
	__m128 Mul06 = _mm_mul_ps(Vec0, Fac1);
	__m128 Mul07 = _mm_mul_ps(Vec1, Fac3);
	__m128 Mul08 = _mm_mul_ps(Vec3, Fac5);
	__m128 Sub02 = _mm_sub_ps(Mul06, Mul07);
	__m128 Add02 = _mm_add_ps(Sub02, Mul08);
	__m128 Inv2 = _mm_mul_ps(SignB, Add02);
    
	// col3
	// - (Vec1[0] * Fac2[0] - Vec1[0] * Fac4[0] + Vec2[0] * Fac5[0]),
	// + (Vec1[0] * Fac2[1] - Vec1[1] * Fac4[1] + Vec2[1] * Fac5[1]),
	// - (Vec1[0] * Fac2[2] - Vec1[2] * Fac4[2] + Vec2[2] * Fac5[2]),
	// + (Vec1[0] * Fac2[3] - Vec1[3] * Fac4[3] + Vec2[3] * Fac5[3]));
	__m128 Mul09 = _mm_mul_ps(Vec0, Fac2);
	__m128 Mul10 = _mm_mul_ps(Vec1, Fac4);
	__m128 Mul11 = _mm_mul_ps(Vec2, Fac5);
	__m128 Sub03 = _mm_sub_ps(Mul09, Mul10);
	__m128 Add03 = _mm_add_ps(Sub03, Mul11);
	__m128 Inv3 = _mm_mul_ps(SignA, Add03);
    
	__m128 Row0 = _mm_shuffle_ps(Inv0, Inv1, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Row1 = _mm_shuffle_ps(Inv2, Inv3, _MM_SHUFFLE(0, 0, 0, 0));
	__m128 Row2 = _mm_shuffle_ps(Row0, Row1, _MM_SHUFFLE(2, 0, 2, 0));
    
	//	valType Determinant = m[0][0] * Inverse[0][0]
	//						+ m[0][1] * Inverse[1][0]
	//						+ m[0][2] * Inverse[2][0]
	//						+ m[0][3] * Inverse[3][0];
    vec4 Vec4Row2;
    Vec4Row2.v = Row2;
    __m128 Det0 = _mm_set1_ps(Dot(Input.Columns[0], Vec4Row2));
	__m128 Rcp0 = _mm_div_ps(_mm_set1_ps(1.0f), Det0);
	//__m128 Rcp0 = _mm_rcp_ps(Det0);
    
	//	Inverse /= Determinant;
    mat4 Result;
    Result.Columns[0].v = _mm_mul_ps(Inv0, Rcp0);
    Result.Columns[1].v = _mm_mul_ps(Inv1, Rcp0);
    Result.Columns[2].v = _mm_mul_ps(Inv2, Rcp0);
    Result.Columns[3].v = _mm_mul_ps(Inv3, Rcp0);
    
    return Result;
};

#else

internal mat4
Mat4Inverse(mat4 m)
{
    f32 Coef00 = m.e[2][2] * m.e[3][3] - m.e[3][2] * m.e[2][3];
    f32 Coef02 = m.e[1][2] * m.e[3][3] - m.e[3][2] * m.e[1][3];
    f32 Coef03 = m.e[1][2] * m.e[2][3] - m.e[2][2] * m.e[1][3];
    
    f32 Coef04 = m.e[2][1] * m.e[3][3] - m.e[3][1] * m.e[2][3];
    f32 Coef06 = m.e[1][1] * m.e[3][3] - m.e[3][1] * m.e[1][3];
    f32 Coef07 = m.e[1][1] * m.e[2][3] - m.e[2][1] * m.e[1][3];
    
    f32 Coef08 = m.e[2][1] * m.e[3][2] - m.e[3][1] * m.e[2][2];
    f32 Coef10 = m.e[1][1] * m.e[3][2] - m.e[3][1] * m.e[1][2];
    f32 Coef11 = m.e[1][1] * m.e[2][2] - m.e[2][1] * m.e[1][2];
    
    f32 Coef12 = m.e[2][0] * m.e[3][3] - m.e[3][0] * m.e[2][3];
    f32 Coef14 = m.e[1][0] * m.e[3][3] - m.e[3][0] * m.e[1][3];
    f32 Coef15 = m.e[1][0] * m.e[2][3] - m.e[2][0] * m.e[1][3];
    
    f32 Coef16 = m.e[2][0] * m.e[3][2] - m.e[3][0] * m.e[2][2];
    f32 Coef18 = m.e[1][0] * m.e[3][2] - m.e[3][0] * m.e[1][2];
    f32 Coef19 = m.e[1][0] * m.e[2][2] - m.e[2][0] * m.e[1][2];
    
    f32 Coef20 = m.e[2][0] * m.e[3][1] - m.e[3][0] * m.e[2][1];
    f32 Coef22 = m.e[1][0] * m.e[3][1] - m.e[3][0] * m.e[1][1];
    f32 Coef23 = m.e[1][0] * m.e[2][1] - m.e[2][0] * m.e[1][1];
    
    vec4 Fac0(Coef00, Coef00, Coef02, Coef03);
    vec4 Fac1(Coef04, Coef04, Coef06, Coef07);
    vec4 Fac2(Coef08, Coef08, Coef10, Coef11);
    vec4 Fac3(Coef12, Coef12, Coef14, Coef15);
    vec4 Fac4(Coef16, Coef16, Coef18, Coef19);
    vec4 Fac5(Coef20, Coef20, Coef22, Coef23);
    
    vec4 Vec0(m.e[1][0], m.e[0][0], m.e[0][0], m.e[0][0]);
    vec4 Vec1(m.e[1][1], m.e[0][1], m.e[0][1], m.e[0][1]);
    vec4 Vec2(m.e[1][2], m.e[0][2], m.e[0][2], m.e[0][2]);
    vec4 Vec3(m.e[1][3], m.e[0][3], m.e[0][3], m.e[0][3]);
    
    vec4 Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
    vec4 Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
    vec4 Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
    vec4 Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);
    
    vec4 SignA(+1, -1, +1, -1);
    vec4 SignB(-1, +1, -1, +1);
    
    mat4 Inverse;
    Inverse.Columns[0] = Inv0 * SignA;
    Inverse.Columns[1] = Inv1 * SignB;
    Inverse.Columns[2] = Inv2 * SignA;
    Inverse.Columns[3] = Inv3 * SignB;
    
    vec4 Row0(Inverse.e[0][0], Inverse.e[1][0], Inverse.e[2][0], Inverse.e[3][0]);
    
    vec4 Dot0(m.Columns[0] * Row0);
    f32 Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);
    
    f32 OneOverDeterminant = (1.0f) / Dot1;
    
    return Inverse * OneOverDeterminant;
}

#endif

internal mat4
Mat4NormalMatrix(mat4 In)
{
    mat4 Result;
    
    Result = Mat4Transpose(Mat4Inverse(In));
    Result.Columns[0].w = 0.0f;
    Result.Columns[1].w = 0.0f;
    Result.Columns[2].w = 0.0f;
    Result.Columns[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    return Result;
}

//Mat2
struct mat2
{
    mat2()
    {
        Columns[0] = vec2(0.0f);
        Columns[1] = vec2(0.0f);
    }
    
    mat2(f32 v)
    {
        e[0][0] = v;
        e[0][1] = 0;
        e[1][0] = 0;
        e[1][1] = v;
    }
    
    union
    {
        f32 e[2][2];
        vec2 Columns[2];
    };
    
};

internal mat2
Mat2Rotation(f32 Radians)
{
    mat2 Result;
    
    f32 CosAngle = cosf(Radians);
    f32 SinAngle = sinf(Radians);
    
    Result.e[0][0] = CosAngle; Result.e[0][1] = -SinAngle;
    Result.e[1][0] = SinAngle; Result.e[1][1] =  CosAngle;
    
    return Result;
}

inline vec2 
operator*(const mat2 &A, const vec2 &B)
{
	return vec2(A.e[0][0]*B.x + A.e[0][1]*B.y,
                A.e[1][0]*B.x + A.e[1][1]*B.y);
}

