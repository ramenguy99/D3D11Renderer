struct quaternion
{
    union {
        struct { f32 x, y, z, w; };
        f32 e[4];
    };
};

inline quaternion
Quaternion(float x, float y, float z, float w)
{
    return {x, y, z, w};
}

inline quaternion
Normalize(quaternion Q)
{
    f32 Magnitude = (f32)sqrt(Q.w * Q.w + Q.x * Q.x + Q.y * Q.y + Q.z * Q.z);
    
    quaternion Result;
    Result.w = Q.w / Magnitude;
    Result.x = Q.x / Magnitude;
    Result.y = Q.y / Magnitude;
    Result.z = Q.z / Magnitude;
    
    return Result;
}


inline mat4
QuaternionToMat4(quaternion Q)
{
    f32 x = Q.x;
    f32 y = Q.y;
    f32 z = Q.z;
    f32 w = Q.w;
    
    float xy = x * y;
    float xz = x * z;
    float xw = x * w;
    float yz = y * z;
    float yw = y * w;
    float zw = z * w;
    float xSquared = x * x;
    float ySquared = y * y;
    float zSquared = z * z;
    
    mat4 Mat;
    Mat.e[0][0] = 1 - 2 * (ySquared + zSquared);
    Mat.e[1][0] = 2 * (xy - zw);
    Mat.e[2][0] = 2 * (xz + yw);
    Mat.e[3][0] = 0;
    
    Mat.e[0][1] = 2 * (xy + zw);
    Mat.e[1][1] = 1 - 2 * (xSquared + zSquared);
    Mat.e[2][1] = 2 * (yz - xw);
    Mat.e[3][1] = 0;
    
    Mat.e[0][2] = 2 * (xz - yw);
    Mat.e[1][2] = 2 * (yz + xw);
    Mat.e[2][2] = 1 - 2 * (xSquared + ySquared);
    Mat.e[3][2] = 0;
    
    Mat.e[0][3] = 0;
    Mat.e[1][3] = 0;
    Mat.e[2][3] = 0;
    Mat.e[3][3] = 1;
    return Mat;
}

internal void
Mat4ToPositionAndQuaternion(mat4 Mat, vec3* Position, quaternion* Rotation) {
    *Position = vec3(Mat.Columns[3]);
    
    float w, x, y, z;
    float diagonal = Mat.e[0][0] + Mat.e[1][1] + Mat.e[2][2];
    if (diagonal > 0) {
        float w4 = (float) (sqrt(diagonal + 1.0f) * 2.0f);
        w = w4 / 4.0f;
        x = (Mat.e[1][2] - Mat.e[2][1]) / w4;
        y = (Mat.e[2][0] - Mat.e[0][2]) / w4;
        z = (Mat.e[0][1] - Mat.e[1][0]) / w4;
    } else if ((Mat.e[0][0] > Mat.e[1][1]) && (Mat.e[0][0] > Mat.e[2][2])) {
        float x4 = (float) (sqrt(1.0f + Mat.e[0][0] - Mat.e[1][1] - Mat.e[2][2]) * 2.0f);
        w = (Mat.e[1][2] - Mat.e[2][1]) / x4;
        x = x4 / 4.0f;
        y = (Mat.e[1][0] + Mat.e[0][1]) / x4;
        z = (Mat.e[2][0] + Mat.e[0][2]) / x4;
    } else if (Mat.e[1][1] > Mat.e[2][2]) {
        float y4 = (float) (sqrt(1.0f + Mat.e[1][1] - Mat.e[0][0] - Mat.e[2][2]) * 2.0f);
        w = (Mat.e[2][0] - Mat.e[0][2]) / y4;
        x = (Mat.e[1][0] + Mat.e[0][1]) / y4;
        y = y4 / 4.0f;
        z = (Mat.e[2][1] + Mat.e[1][2]) / y4;
    } else {
        float z4 = (float) (sqrt(1.0f + Mat.e[2][2] - Mat.e[0][0] - Mat.e[1][1]) * 2.0f);
        w = (Mat.e[0][1] - Mat.e[1][0]) / z4;
        x = (Mat.e[2][0] + Mat.e[0][2]) / z4;
        y = (Mat.e[2][1] + Mat.e[1][2]) / z4;
        z = z4 / 4.0f;
    }
    
    *Rotation = {x, y, z, w};
}

//nlerp interpolation
internal quaternion
Interpolate(quaternion A, quaternion B, float Mix)
{
    quaternion Result = Quaternion(0, 0, 0, 1);
    
    float Dot = A.w * B.w + A.x * B.x + A.y * B.y + A.z * B.z;
    float MixI = 1.0f - Mix;
    if (Dot < 0) {
        Result.w = MixI * A.w + Mix * -B.w;
        Result.x = MixI * A.x + Mix * -B.x;
        Result.y = MixI * A.y + Mix * -B.y;
        Result.z = MixI * A.z + Mix * -B.z;
    } else {
        Result.w = MixI * A.w + Mix * B.w;
        Result.x = MixI * A.x + Mix * B.x;
        Result.y = MixI * A.y + Mix * B.y;
        Result.z = MixI * A.z + Mix * B.z;
    }
    Result = Normalize(Result);
    return Result;
}