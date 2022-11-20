internal f64 
AreaElement(f32 x, f32 y)
{
    return atan2(x * y, sqrt(x * x + y * y + 1));
}

internal f64 
TexelSolidAngle(f32 U, f32 V, f32 InvResolution)
{
    // U and V are the -1..1 texture coordinate on the current face.
    // Get projected area for this texel
    float32 x0 = U - InvResolution;
    float32 y0 = V - InvResolution;
    float32 x1 = U + InvResolution;
    float32 y1 = V + InvResolution;
    f64 SolidAngle = AreaElement(x0, y0) - AreaElement(x0, y1) - AreaElement(x1, y0) + AreaElement(x1, y1);
    
    return SolidAngle;
}

internal void
SumWeightedSphericalHarmonics(f64* SH, vec3 n, f64 domega, float L)
{
    f64 x = n.x;
    f64 y = n.y;
    f64 z = n.z;
    
    f64 c; /* A different constant for each coefficient */
    
    // L00 L11 L10 L1n1 L22 L21 L20 L2n1 L2n2
    //  0   1   2    3   4   5   6    7    8
    
    /* L_{00}.  Note that Y_{00} = 0.282095 */
    c = 0.282095;
    SH[0] += L*c*domega;
    
    /* L_{1m}. -1 <= m <= 1.  The linear terms */
    c = 0.488603;
    SH[1] += L*(c*x)*domega;   /* Y_{11}  = 0.488603 x  */
    SH[2] += L*(c*z)*domega;   /* Y_{10}  = 0.488603 z  */
    SH[3] += L*(c*y)*domega;   /* Y_{1-1} = 0.488603 y  */
    
    /* The Quadratic terms, L_{2m} -2 <= m <= 2 */
    
    /* First, L_{2-2}, L_{2-1}, L_{21} corresponding to xy,yz,xz */
    c = 1.092548;
    SH[8] += L*(c*x*y)*domega; /* Y_{2-2} = 1.092548 xy */ 
    SH[7] += L*(c*y*z)*domega; /* Y_{2-1} = 1.092548 yz */ 
    SH[5] += L*(c*x*z)*domega; /* Y_{21}  = 1.092548 xz */ 
    
    /* L_{20}.  Note that Y_{20} = 0.315392 (3z^2 - 1) */
    c = 0.315392;
    SH[6] += L*(c*(3*z*z-1))*domega; 
    
    /* L_{22}.  Note that Y_{22} = 0.546274 (x^2 - y^2) */
    c = 0.546274;
    SH[4] += L*(c*(x*x-y*y))*domega;
}

internal void
ComputeSphericalHarmonics(HANDLE File, asset_table Table, char* Name)
{
    asset_table_entry* Entry = FindAsset(Name, Table, ASSET_CUBEMAP, "environment");
    
    void* Data = ZeroAlloc(Entry->Size);
    Win32_ReadAtOffset(File, Data, Entry->Size, Entry->Offset);
    cubemap_data CubemapData = LoadCubemapAsset(Data, Entry->Size);
    
    //All sizes in bytes
    s32 FaceSize = CubemapData.Pitch * CubemapData.Size;
    s32 RowSize = CubemapData.Pitch;
    s32 PixelSize = CubemapData.BytesPerPixel;
    
    vec3 Vz[6] = {
        vec3( 1,  0,  0),
        vec3(-1,  0,  0),
        vec3( 0,  1,  0),
        vec3( 0, -1,  0),
        vec3( 0,  0,  1),
        vec3( 0,  0, -1),
    };
    
    vec3 Vx[6] = {
        vec3( 0,  0, -1),
        vec3( 0,  0,  1),
        vec3( 1,  0,  0),
        vec3( 1,  0,  0),
        vec3( 1,  0,  0),
        vec3(-1,  0,  0),
    };
    
    vec3 Vy[6] = {
        vec3( 0, -1,  0),
        vec3( 0, -1,  0),
        vec3( 0,  0,  1),
        vec3( 0,  0, -1),
        vec3( 0, -1,  0),
        vec3( 0, -1,  0),
    };
    
    f64 SH_R[9] = {};
    f64 SH_G[9] = {};
    f64 SH_B[9] = {};
    
    f64 TotalW = 0.0f;
    
    f32 InvResolution = 1.0f / CubemapData.Size;
    for(s32 FaceIndex = 0; FaceIndex < 6; FaceIndex++)
    {
        u8* FaceBegin = CubemapData.Data + FaceSize * FaceIndex;
        
        for(s32 y = 0; y < CubemapData.Size; y++)
        {
            u8* RowBegin = FaceBegin + RowSize * y;
            for(s32 x = 0; x < CubemapData.Size; x++)
            {
                vec4 L = *(vec4*)(RowBegin + PixelSize * x);
                
                float u = (x + 0.5f) / CubemapData.Size * 2.0f - 1.0f;
                float v = (y + 0.5f) / CubemapData.Size * 2.0f - 1.0f;
                float w = 1.0f;
                
                //Equivalent to Mx where x = vec3(u,v,w) and M = [Vx, Vy, Vz] as columns
                vec3 n = Vx[FaceIndex] * u + Vy[FaceIndex] * v + Vz[FaceIndex] * w;
                n = Normalize(n);
                
                f64 SolidAngle = TexelSolidAngle(u, v, InvResolution);
                Assert(SolidAngle >= 0.0f);
                
                SumWeightedSphericalHarmonics(SH_R, n, SolidAngle, L.r);
                SumWeightedSphericalHarmonics(SH_G, n, SolidAngle, L.g);
                SumWeightedSphericalHarmonics(SH_B, n, SolidAngle, L.b);
                
                TotalW += SolidAngle;
            }
        }
    }
    
    f64 InvTotalW = 1.0 / TotalW;
    for(int i = 0; i < 9; i++)
    {
        InspectorData.L[i] = 
            vec4(
                 (f32)(SH_R[i] * InvTotalW), 
                 (f32)(SH_G[i] * InvTotalW), 
                 (f32)(SH_B[i] * InvTotalW), 
                 1.0f);
    }
}
