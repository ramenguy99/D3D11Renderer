#include "direct3d11.h"

internal ID3D11VertexShader*
D3D11_LoadVertexShader(ID3D11Device* Device, wchar_t* Path, ID3D10Blob** OutBuffer)
{
    ID3D11VertexShader* Result = 0;
    HRESULT HResult;
    
    ID3D10Blob* Buffer = 0;
    ID3D10Blob* ErrorMessage = 0;
    
    retry:
    HResult = D3DCompileFromFile(Path, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                 "VertexMain", "vs_5_0",
                                 D3D10_SHADER_ENABLE_STRICTNESS, 0,
                                 &Buffer, &ErrorMessage);
    if(HResult != S_OK) {
        if(!ErrorMessage)
            goto retry;
        char* ErrorData = (char*)ErrorMessage->GetBufferPointer();
        Assert(0);
    }
    
    HResult = Device->CreateVertexShader(Buffer->GetBufferPointer(),
                                         Buffer->GetBufferSize(), 0, &Result);
    Assert(HResult == S_OK);
    
    if(OutBuffer)
    {
        *OutBuffer = Buffer;
    }
    else
    {
        Buffer->Release();
    }
    
    return Result;
}

internal ID3D11PixelShader*
D3D11_LoadPixelShader(ID3D11Device* Device, wchar_t* Path)
{
    ID3D11PixelShader* Result = 0;
    HRESULT HResult;
    
    ID3D10Blob* Buffer = 0;
    ID3D10Blob* ErrorMessage = 0;
    
    retry:
    HResult = D3DCompileFromFile(Path, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                 "PixelMain", "ps_5_0",
                                 D3D10_SHADER_ENABLE_STRICTNESS, 0,
                                 &Buffer, &ErrorMessage);
    if(HResult != S_OK) {
        if(!ErrorMessage)
            goto retry;
        char* ErrorData = (char*)ErrorMessage->GetBufferPointer();
        Assert(0);
    }
    
    HResult = Device->CreatePixelShader(Buffer->GetBufferPointer(),
                                        Buffer->GetBufferSize(), 0, &Result);
    Assert(HResult == S_OK);
    Buffer->Release();
    return Result;
}

internal ID3D11GeometryShader*
D3D11_LoadGeometryShader(ID3D11Device* Device, wchar_t* Path)
{
    ID3D11GeometryShader* Result = 0;
    HRESULT HResult;
    
    ID3D10Blob* Buffer = 0;
    ID3D10Blob* ErrorMessage = 0;
    
    retry:
    HResult = D3DCompileFromFile(Path, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                 "GeometryMain", "gs_5_0",
                                 D3D10_SHADER_ENABLE_STRICTNESS, 0,
                                 &Buffer, &ErrorMessage);
    if(HResult != S_OK) {
        if(!ErrorMessage)
            goto retry;
        //Assert(ErrorMessage); //If no error message we probably didn't find a fail (check HRESULT)
        char* ErrorData = (char*)ErrorMessage->GetBufferPointer();
        Assert(0);
    }
    
    HResult = Device->CreateGeometryShader(Buffer->GetBufferPointer(),
                                           Buffer->GetBufferSize(), 0, &Result);
    Assert(HResult == S_OK);
    Buffer->Release();
    return Result;
}

internal d3d11_render_target
D3D11_CreateRenderTarget(ID3D11Device* Device, u32 Width, u32 Height, DXGI_FORMAT TextureFormat, 
                         DXGI_FORMAT TargetFormat, DXGI_FORMAT ResourceFormat)
{
    d3d11_render_target Result = {};
    Result.Width = Width;
    Result.Height = Height;
    
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = Width;
    TextureDesc.Height = Height;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = TextureFormat;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    
    HRESULT HResult = Device->CreateTexture2D(&TextureDesc, 0, &Result.Texture);
    Assert(HResult == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC ResourceDesc = {};
    ResourceDesc.Format = ResourceFormat;
    ResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ResourceDesc.Texture2D.MipLevels = (u32)-1;
    
    HResult = Device->CreateShaderResourceView(Result.Texture, &ResourceDesc, &Result.ResourceView);
    Assert(HResult == S_OK);
    
    D3D11_RENDER_TARGET_VIEW_DESC RenderTargetDesc = {};
    RenderTargetDesc.Format = TargetFormat;
    RenderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    
    HResult = Device->CreateRenderTargetView(Result.Texture, &RenderTargetDesc, &Result.RenderTarget);
    Assert(HResult == S_OK);
    
    return Result;
}

internal d3d11_render_target_3d
D3D11_CreateRenderTarget3D(ID3D11Device* Device, u32 Width, u32 Height, u32 Depth, DXGI_FORMAT Format)
{
    d3d11_render_target_3d Result = {};
    Result.Width = Width;
    Result.Height = Height;
    Result.Depth = Depth;
    
    D3D11_TEXTURE3D_DESC TextureDesc = {};
    TextureDesc.Width = Width;
    TextureDesc.Height = Height;
    TextureDesc.Depth = Depth;
    TextureDesc.MipLevels = 1;
    TextureDesc.Format = Format;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    
    HRESULT HResult = Device->CreateTexture3D(&TextureDesc, 0, &Result.Texture);
    Assert(HResult == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC ResourceDesc = {};
    ResourceDesc.Format = Format;
    ResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    ResourceDesc.Texture3D.MipLevels = (u32)-1;
    HResult = Device->CreateShaderResourceView(Result.Texture, &ResourceDesc, &Result.ResourceView);
    Assert(HResult == S_OK);
    
    
    D3D11_RENDER_TARGET_VIEW_DESC RenderTargetDesc = {};
    RenderTargetDesc.Format = Format;
    RenderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
    RenderTargetDesc.Texture3D.MipSlice = 0;
    RenderTargetDesc.Texture3D.FirstWSlice = 0;
    RenderTargetDesc.Texture3D.WSize = Depth;
    
    HResult = Device->CreateRenderTargetView(Result.Texture, &RenderTargetDesc, &Result.RenderTarget);
    Assert(HResult == S_OK);
    
    return Result;
}

internal d3d11_render_target
D3D11_CreateRenderTarget(ID3D11Device* Device, u32 Width, u32 Height, DXGI_FORMAT Format)
{
    return D3D11_CreateRenderTarget(Device, Width, Height, Format, Format, Format);
}

internal d3d11_render_target
D3D11_CreateRenderTargetLDR(ID3D11Device* Device, u32 Width, u32 Height,
                            b32 TargetSRGB = false, b32 ResourceSRGB = false)
{
    DXGI_FORMAT TargetFormat = TargetSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT ResourceFormat = ResourceSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
    return D3D11_CreateRenderTarget(Device, Width, Height, DXGI_FORMAT_R8G8B8A8_TYPELESS,
                                    TargetFormat, ResourceFormat);
}

internal void
D3D11_FreeRenderTarget(d3d11_render_target* RenderTarget)
{
    RenderTarget->ResourceView->Release();
    RenderTarget->Texture->Release();
    RenderTarget->RenderTarget->Release();
    *RenderTarget = {};
}

internal void
D3D11_FillConstantBuffers(ID3D11DeviceContext* Context, ID3D11Buffer* Buffer, void* Data, u32 Size)
{
    D3D11_MAPPED_SUBRESOURCE Resource = {};
    Context->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
    memcpy(Resource.pData, Data, Size);
    Context->Unmap(Buffer, 0);
}

internal ID3D11Buffer*
D3D11_CreateConstantBuffer(ID3D11Device* Device, u32 Size)
{
    D3D11_BUFFER_DESC ConstantDesc = {};
    ConstantDesc.ByteWidth = ALIGN_UP(Size, 16);
    ConstantDesc.Usage = D3D11_USAGE_DYNAMIC;
    ConstantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    ConstantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    ID3D11Buffer* ConstantBuffer = 0;
    HRESULT Result = Device->CreateBuffer(&ConstantDesc, 0, &ConstantBuffer);
    Assert(Result == S_OK);
    
    return ConstantBuffer;
}

internal ID3D11Buffer*
D3D11_CreateBuffer(ID3D11Device* Device, void* Data, u32 Size, bool IsIndexBuffer = false)
{
    D3D11_SUBRESOURCE_DATA Resource = {};
    Resource.pSysMem = Data;
    
    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.ByteWidth = Size;
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.BindFlags = IsIndexBuffer ? D3D11_BIND_INDEX_BUFFER : D3D11_BIND_VERTEX_BUFFER;
    
    ID3D11Buffer* Buffer;
    HRESULT HResult = Device->CreateBuffer(&BufferDesc, &Resource, &Buffer);
    Assert(HResult == S_OK);
    
    return Buffer;
}

internal ID3D11Buffer*
D3D11_CreateMappableBuffer(ID3D11Device* Device, u32 Size)
{
    //Triangle buffer
    D3D11_BUFFER_DESC BufferDesc = {};
    BufferDesc.ByteWidth = Size;
    BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    ID3D11Buffer* Buffer;
    HRESULT Result = Device->CreateBuffer(&BufferDesc, 0, &Buffer);
    Assert(Result == S_OK);
    
    return Buffer;
}
    

internal d3d11_profiler
D3D11_InitProfiler(ID3D11Device* Device)
{
    d3d11_profiler Result = {};
    
    HRESULT HResult;
    
    D3D11_QUERY_DESC Desc = {};
    
    for(u32 Frame = 0; Frame < ArrayCount(Result.Frames); Frame++)
    {
        Desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
        HResult = Device->CreateQuery(&Desc, &Result.Frames[Frame].QueryDisjoint);
        Assert(HResult == S_OK);
        
        Desc.Query = D3D11_QUERY_TIMESTAMP;
        HResult = Device->CreateQuery(&Desc, &Result.Frames[Frame].QueryBeginFrame);
        Assert(HResult == S_OK);
        HResult = Device->CreateQuery(&Desc, &Result.Frames[Frame].QueryEndFrame);
        Assert(HResult == S_OK);
        
        for(u32 i = 0; i < D3D11_PROFILE_AREAS_COUNT; i++)
        {
            HResult = Device->CreateQuery(&Desc, Result.Frames[Frame].BeginQueries + i);
            Assert(HResult == S_OK);
            HResult = Device->CreateQuery(&Desc, Result.Frames[Frame].EndQueries + i);
            Assert(HResult == S_OK);
        }
    }
    
    return Result;
}

internal void
D3D11_ProfilerBeginFrame(d3d11_state* D3D11)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    d3d11_profiler* Profiler = &D3D11->Profiler;
    
    Context->Begin(Profiler->Frames[Profiler->CurrentFrame].QueryDisjoint);
    Context->End(Profiler->Frames[Profiler->CurrentFrame].QueryBeginFrame);
}

internal void
D3D11_ProfilerEndFrame(d3d11_state* D3D11)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    d3d11_profiler* Profiler = &D3D11->Profiler;
    
    Context->End(Profiler->Frames[Profiler->CurrentFrame].QueryEndFrame);
    Context->End(Profiler->Frames[Profiler->CurrentFrame].QueryDisjoint);
    
    //Advance current frame
    Profiler->CurrentFrame = (Profiler->CurrentFrame + 1) % ArrayCount(Profiler->Frames);
    
    //Skip the first couple of frames to enable buffering
    static b32 FramesToSkip = ArrayCount(Profiler->Frames) - 1;
    if(FramesToSkip > 0) {
        FramesToSkip--;
        return;
    }
    
    //Collect data for the available frame
    d3d11_profiler_frame* Frame = Profiler->Frames + Profiler->CurrentFrame;
    
    // Wait for data to be available
    while (Context->GetData(Frame->QueryDisjoint, NULL, 0, 0) == S_FALSE)
    {
        Assert(0);
        Sleep(1);
    }
    
    // Check whether timestamps were disjoint during the last frame
    D3D10_QUERY_DATA_TIMESTAMP_DISJOINT Disjoint = {};
    HRESULT HResult = Context->GetData(Frame->QueryDisjoint, &Disjoint, sizeof(Disjoint), 0);
    
    if(!Disjoint.Disjoint)
    {
        Profiler->FrameIsValid = true;
        f32 Frequency = (f32)Disjoint.Frequency;
        
        u64 FrameBegin, FrameEnd;
        HResult = Context->GetData(Frame->QueryBeginFrame, &FrameBegin, sizeof(u64), 0);
        HResult = Context->GetData(Frame->QueryEndFrame, &FrameEnd, sizeof(u64), 0);
        
        Profiler->FrameTime = (FrameEnd - FrameBegin) / Frequency;
        
        for(u32 i = 0; i < D3D11_PROFILE_AREAS_COUNT; i++) 
        {
            u64 Begin, End;
            Context->GetData(Frame->BeginQueries[i], &Begin, sizeof(u64), 0);
            Context->GetData(Frame->EndQueries[i], &End, sizeof(u64), 0);
            
            Profiler->AreaTime[i] = (End - Begin) / Frequency;
        }
    } 
    else 
    {
        Profiler->FrameIsValid = false;
    }
    
}

internal void
D3D11_ProfilerBeginArea(d3d11_state* D3D11, d3d11_profiler_area Area)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    d3d11_profiler* Profiler = &D3D11->Profiler;
    
    Assert(Area < D3D11_PROFILE_AREAS_COUNT);
    Context->End(Profiler->Frames[Profiler->CurrentFrame].BeginQueries[Area]);
}

internal void
D3D11_ProfilerEndArea(d3d11_state* D3D11, d3d11_profiler_area Area)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    d3d11_profiler* Profiler = &D3D11->Profiler;
    
    Assert(Area < D3D11_PROFILE_AREAS_COUNT);
    Context->End(Profiler->Frames[Profiler->CurrentFrame].EndQueries[Area]);
}

internal d3d11_common
D3D11_InitCommonState(ID3D11Device* Device)
{
    HRESULT Result;
    
    //DepthStencil State
    D3D11_DEPTH_STENCIL_DESC DepthStencilStateDesc = {};
    DepthStencilStateDesc.DepthEnable = true;
	DepthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    DepthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DepthStencilStateDesc.StencilEnable = false;
    
    //Less
    ID3D11DepthStencilState* LessDepthStencilState = 0;
    Result = Device->CreateDepthStencilState(&DepthStencilStateDesc, &LessDepthStencilState);
    Assert(Result == S_OK);
    
    //Greater
    DepthStencilStateDesc.DepthFunc = D3D11_COMPARISON_GREATER;
    ID3D11DepthStencilState* GreaterDepthStencilState = 0;
    Result = Device->CreateDepthStencilState(&DepthStencilStateDesc, &GreaterDepthStencilState);
    Assert(Result == S_OK);
    
    //Equal
    DepthStencilStateDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
    ID3D11DepthStencilState* EqualDepthStencilState = 0;
    Result = Device->CreateDepthStencilState(&DepthStencilStateDesc, &EqualDepthStencilState);
    Assert(Result == S_OK);
    
    
    //Rasterizer
    D3D11_RASTERIZER_DESC RasterDesc = {};
    RasterDesc.FillMode = D3D11_FILL_SOLID;
    RasterDesc.CullMode = D3D11_CULL_BACK;
    RasterDesc.FrontCounterClockwise = MESH_WINDING_COUNTER_CLOCKWISE;
    RasterDesc.DepthClipEnable = true;
        
    ID3D11RasterizerState* SolidRasterState = 0;
    Result = Device->CreateRasterizerState(&RasterDesc, &SolidRasterState);
    Assert(Result == S_OK);
    
    RasterDesc.CullMode = D3D11_CULL_FRONT;
    ID3D11RasterizerState* ShadowRasterState = 0;
    Result = Device->CreateRasterizerState(&RasterDesc, &ShadowRasterState);
    Assert(Result == S_OK);
    
    RasterDesc.CullMode = D3D11_CULL_NONE;
    ID3D11RasterizerState* SolidRasterStateNoCull = 0;
    Result = Device->CreateRasterizerState(&RasterDesc, &SolidRasterStateNoCull);
    Assert(Result == S_OK);
    
    RasterDesc.FillMode = D3D11_FILL_WIREFRAME;
    ID3D11RasterizerState* WireRasterState = 0;
    Result = Device->CreateRasterizerState(&RasterDesc, &WireRasterState);
    Assert(Result == S_OK);
    
    //Sampler
    D3D11_SAMPLER_DESC SamplerDesc = {};
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    SamplerDesc.MaxAnisotropy = 1;
    SamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    SamplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
    SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    ID3D11SamplerState* LinearSamplerState = 0;
    Result = Device->CreateSamplerState(&SamplerDesc, &LinearSamplerState);
    Assert(Result == S_OK);
    
    ID3D11SamplerState* PointSamplerState = 0;
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    Result = Device->CreateSamplerState(&SamplerDesc, &PointSamplerState);
    Assert(Result == S_OK);
    
    SamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    SamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    SamplerDesc.MaxAnisotropy = 16;
    ID3D11SamplerState* AnisotropicSamplerState = 0;
    Result = Device->CreateSamplerState(&SamplerDesc, &AnisotropicSamplerState);
    Assert(Result == S_OK);
    
    D3D11_SAMPLER_DESC ShadowSamplerDesc = {};
    ShadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    ShadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    ShadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    ShadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    ShadowSamplerDesc.BorderColor[0] = 1.0f;
    ShadowSamplerDesc.BorderColor[1] = 1.0f;
    ShadowSamplerDesc.BorderColor[2] = 1.0f;
    ShadowSamplerDesc.BorderColor[3] = 1.0f;
    ShadowSamplerDesc.MaxAnisotropy = 1;
    ShadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    ShadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ID3D11SamplerState* ShadowSamplerState = 0;
    Result = Device->CreateSamplerState(&ShadowSamplerDesc, &ShadowSamplerState);
    Assert(Result == S_OK);
    
    //Frame vertex shader
    ID3D10Blob* FrameVertexShaderBuffer;
    ID3D11VertexShader* FrameVertexShader = D3D11_LoadVertexShader(Device, L"../src/shaders/frame_vertex.hlsl", 
                                                                   &FrameVertexShaderBuffer);
    //Frame input layout
    D3D11_INPUT_ELEMENT_DESC FrameLayoutDesc[2] = {};
    FrameLayoutDesc[0].SemanticName = "POSITION";
    FrameLayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    FrameLayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    
    FrameLayoutDesc[1].SemanticName = "TEXCOORD";
    FrameLayoutDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    FrameLayoutDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    FrameLayoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    
    ID3D11InputLayout* FrameLayout = 0;
    Result = Device->CreateInputLayout(FrameLayoutDesc, ArrayCount(FrameLayoutDesc),
                                           FrameVertexShaderBuffer->GetBufferPointer(), 
                                           FrameVertexShaderBuffer->GetBufferSize(),  &FrameLayout);
    Assert(Result == S_OK);
    FrameVertexShaderBuffer->Release();
    
    struct { 
        vec3 P;
        vec2 UV;
    } FrameVertices[] = {
        {vec3(-1.0f, 1.0f, 0.0f), vec2(0.0f, 0.0f)},
        {vec3( 1.0f, 1.0f, 0.0f), vec2(1.0f, 0.0f)},
        {vec3( 1.0f,-1.0f, 0.0f), vec2(1.0f, 1.0f)},
        {vec3( 1.0f,-1.0f, 0.0f), vec2(1.0f, 1.0f)},
        {vec3(-1.0f,-1.0f, 0.0f), vec2(0.0f, 1.0f)},
        {vec3(-1.0f, 1.0f, 0.0f), vec2(0.0f, 0.0f)},
    };
    Assert(sizeof(FrameVertices[0]) == sizeof(vec3) + sizeof(vec2));
    ID3D11Buffer* FrameVertexBuffer = D3D11_CreateBuffer(Device, FrameVertices, sizeof(FrameVertices));
    
    //Cubemap
    ID3D11Buffer* CubemapVertexBuffer = D3D11_CreateBuffer(Device, GlobalCubePositions, sizeof(GlobalCubePositions));
    ID3D11Buffer* CubemapVertexConstantsBuffer = D3D11_CreateConstantBuffer(Device, sizeof(d3d11_cubemap_vertex_constants));
    ID3D11Buffer* CubemapPixelConstantsBuffer = D3D11_CreateConstantBuffer(Device, sizeof(d3d11_cubemap_pixel_constants));
    
    ID3D10Blob* CubemapVertexShaderBuffer;
    ID3D11VertexShader* CubemapVertexShader = D3D11_LoadVertexShader(Device, L"../src/shaders/cubemap.hlsl", &CubemapVertexShaderBuffer);
    D3D11_INPUT_ELEMENT_DESC CubemapLayoutDesc[1] = {};
    CubemapLayoutDesc[0].SemanticName = "POSITION";
    CubemapLayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    CubemapLayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    ID3D11InputLayout* CubemapLayout;
    Result = Device->CreateInputLayout(CubemapLayoutDesc, ArrayCount(CubemapLayoutDesc),
                                           CubemapVertexShaderBuffer->GetBufferPointer(),
                                           CubemapVertexShaderBuffer->GetBufferSize(), &CubemapLayout);
    Assert(Result == S_OK);
    CubemapVertexShaderBuffer->Release();
    
    ID3D11PixelShader* CubemapPixelShader = D3D11_LoadPixelShader(Device, L"../src/shaders/cubemap.hlsl");
    
    //Sphere
    mesh_data Sphere = AllocSphereMesh(0.5);
    ID3D11Buffer* SphereVertexBuffer = D3D11_CreateBuffer(Device, Sphere.Positions, sizeof(vec3) * Sphere.VerticesCount);
    ID3D11Buffer* SphereIndexBuffer = D3D11_CreateBuffer(Device, Sphere.Indices, sizeof(u32) * Sphere.IndicesCount, true);
    u32 SphereIndicesCount = Sphere.IndicesCount;
    FreeMesh(&Sphere);
    
    //3D texture slice
    ID3D11Buffer* Texture3DSliceConstantsBuffer = D3D11_CreateConstantBuffer(Device, sizeof(f32));
    ID3D11PixelShader* Texture3DSlicePixelShader = D3D11_LoadPixelShader(Device, L"../src/shaders/texture3d_slice.hlsl");
    
    
    d3d11_common Common = {};
    Common.WireRasterState = WireRasterState;
    Common.SolidRasterState = SolidRasterState;
    Common.SolidRasterStateNoCull = SolidRasterStateNoCull;
    Common.LessDepthStencilState = LessDepthStencilState;
    Common.GreaterDepthStencilState = GreaterDepthStencilState;
    Common.EqualDepthStencilState = EqualDepthStencilState;
    Common.LinearSamplerState = LinearSamplerState;
    Common.AnisotropicSamplerState = AnisotropicSamplerState;
    Common.PointSamplerState = PointSamplerState;
    Common.ShadowSamplerState = ShadowSamplerState;
    Common.FrameLayout = FrameLayout;
    Common.FrameVertexShader = FrameVertexShader;
    Common.FrameVertexBuffer = FrameVertexBuffer;
    Common.SphereVertexBuffer = SphereVertexBuffer;
    Common.SphereIndexBuffer = SphereIndexBuffer;
    Common.SphereIndicesCount = SphereIndicesCount;
    Common.CubemapVertexBuffer = CubemapVertexBuffer;
    Common.CubemapVertexShader = CubemapVertexShader;
    Common.CubemapPixelShader = CubemapPixelShader;
    Common.CubemapLayout = CubemapLayout;
    Common.CubemapVertexConstantsBuffer = CubemapVertexConstantsBuffer;
    Common.CubemapPixelConstantsBuffer = CubemapPixelConstantsBuffer;
    Common.Texture3DSliceConstantsBuffer = Texture3DSliceConstantsBuffer;
    Common.Texture3DSlicePixelShader = Texture3DSlicePixelShader;
    
    return Common;
};

internal d3d11_texture
D3D11_ComputeBRDF(d3d11_state* D3D11)
{
    u32 Size = BRDF_SIZE;
    ID3D11Device* Device = D3D11->Device;
    ID3D11DeviceContext* Context = D3D11->Context;
    
    HRESULT Result;
    
    //Draw BRDF Map
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = Size;
    TextureDesc.Height = Size;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    
    ID3D11Texture2D* Texture = 0;
    Result = Device->CreateTexture2D(&TextureDesc, 0, &Texture);
    Assert(Result == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC TextureResourceDesc = {};
    TextureResourceDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
    TextureResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    TextureResourceDesc.Texture2D.MipLevels = (u32)-1;
    
    ID3D11ShaderResourceView* TextureResource = 0;
    Result = Device->CreateShaderResourceView(Texture, &TextureResourceDesc, &TextureResource);
    Assert(Result == S_OK);
    
    D3D11_RENDER_TARGET_VIEW_DESC RenderTargetDesc = {};
    RenderTargetDesc.Format = DXGI_FORMAT_R16G16_FLOAT;
    RenderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    RenderTargetDesc.Texture2D.MipSlice = 0;
    ID3D11RenderTargetView* RenderTarget = 0;
    Result = Device->CreateRenderTargetView(Texture, &RenderTargetDesc, &RenderTarget);
    Assert(Result == S_OK);
    
    ID3D11PixelShader* PixelShader = D3D11_LoadPixelShader(Device, L"../src/shaders/brdf_gen.hlsl");
    
    //Viewport
    D3D11_VIEWPORT Viewport = {};
    Viewport.Width = (float)Size;
    Viewport.Height = (float)Size;
    Viewport.MaxDepth = 1.0f;
    
    Context->ClearState();
    Context->ClearRenderTargetView(RenderTarget, vec4(1.0f).e);
    Context->OMSetRenderTargets(1, &RenderTarget, 0);
    Context->OMSetDepthStencilState(D3D11->Common.LessDepthStencilState, 0);
    Context->RSSetState(D3D11->Common.SolidRasterStateNoCull);
    Context->RSSetViewports(1, &Viewport);
    u32 Strides[] = {sizeof(vec3) + sizeof(vec2)};
    u32 Offsets[] = {0};
    Context->IASetVertexBuffers(0, 1, &D3D11->Common.FrameVertexBuffer, Strides, Offsets);
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(D3D11->Common.FrameLayout);
    Context->VSSetShader(D3D11->Common.FrameVertexShader, 0, 0);
    Context->PSSetShader(PixelShader, 0, 0);
    
    Context->Draw(6, 0);
    
    RenderTarget->Release();
    
    d3d11_texture BRDF;
    BRDF.ResourceView = TextureResource;
    BRDF.Texture = Texture;
    
    return BRDF;
}

internal d3d11_pbr_pipeline
D3D11_InitPBRPipeline(d3d11_state* D3D11, u32 Width, u32 Height)
{
    ID3D11Device* Device = D3D11->Device;
    
    d3d11_pbr_pipeline PBR = {};
    ID3D10Blob* Buffer;
    PBR.VertexShader = D3D11_LoadVertexShader(Device, L"../src/shaders/pbr_vertex.hlsl", &Buffer);
    PBR.PixelShader  = D3D11_LoadPixelShader(Device, L"../src/shaders/pbr_pixel.hlsl");
        
    D3D11_INPUT_ELEMENT_DESC LayoutDesc[4] = {};
    LayoutDesc[0].SemanticName = "POSITION";
    LayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    LayoutDesc[0].InputSlot = 0;
    LayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    
    LayoutDesc[1].SemanticName = "NORMAL";
    LayoutDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    LayoutDesc[1].InputSlot = 1;
    LayoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    
    LayoutDesc[2].SemanticName = "TANGENT";
    LayoutDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    LayoutDesc[2].InputSlot = 2;
    LayoutDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    
    LayoutDesc[3].SemanticName = "TEXCOORD";
    LayoutDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
    LayoutDesc[3].InputSlot = 3;
    LayoutDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    HRESULT HResult = Device->CreateInputLayout(LayoutDesc, ArrayCount(LayoutDesc),
                                                    Buffer->GetBufferPointer(),
                                                    Buffer->GetBufferSize(), &PBR.Layout);
    Assert(HResult == S_OK);
    Buffer->Release();
    
    PBR.VertexConstantsBuffer = D3D11_CreateConstantBuffer(Device, sizeof(d3d11_pbr_vertex_constants));
    PBR.PixelConstantsBuffer  = D3D11_CreateConstantBuffer(Device, sizeof(d3d11_pbr_pixel_constants));
    
    // This is now stored in the asset file to speed up startup
    // PBR.BRDFTexture = D3D11_ComputeBRDF(D3D11);
    
    // Old LDR pipeline
    // PBR.IntermediateTarget = D3D11_CreateRenderTargetLDR(Device, Width, Height, true, true);
    
    PBR.IntermediateTarget = D3D11_CreateRenderTarget(Device, Width, Height, DXGI_FORMAT_R16G16B16A16_FLOAT);
    PBR.PostprocessShader  = D3D11_LoadPixelShader(Device, L"../src/shaders/postprocess.hlsl");
    
    return PBR;
}

internal d3d11_precompute_pipeline
D3D11_InitPrecomputePipeline(d3d11_state* D3D11)
{
    ID3D11Device* Device = D3D11->Device;
    
    d3d11_precompute_pipeline Precompute = {};
    ID3D10Blob* Buffer;
    Precompute.VertexShader = D3D11_LoadVertexShader(Device, L"../src/shaders/precompute_vertex.hlsl", &Buffer);
    
    D3D11_INPUT_ELEMENT_DESC LayoutDesc[1] = {};
    LayoutDesc[0].SemanticName = "POSITION";
    LayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    LayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    HRESULT HResult = D3D11->Device->CreateInputLayout(LayoutDesc, ArrayCount(LayoutDesc),
                                                           Buffer->GetBufferPointer(),
                                                           Buffer->GetBufferSize(), &Precompute.Layout);
    Assert(HResult == S_OK);
    
    Precompute.EquirectPixelShader = D3D11_LoadPixelShader(Device, L"../src/shaders/equirect.hlsl");
    Precompute.IrradiancePixelShader = D3D11_LoadPixelShader(Device, L"../src/shaders/irradiance_cube.hlsl");
    Precompute.SpecularPixelShader = D3D11_LoadPixelShader(Device, L"../src/shaders/specular_cube.hlsl");
    
    Precompute.VertexConstantsBuffer = D3D11_CreateConstantBuffer(Device, sizeof(d3d11_precompute_vertex_constants));
    Precompute.PixelConstantsBuffer = D3D11_CreateConstantBuffer(Device, sizeof(d3d11_precompute_pixel_constants));
    
    return Precompute;
};

internal d3d11_atmosphere_pipeline
D3D11_InitAtmospherePipeline(d3d11_state* D3D11)
{
    ID3D11Device* Device = D3D11->Device;
    
    d3d11_atmosphere_pipeline Atmosphere = {};
    
    
    // We now store the precomputed atmosphere in the .atmo so we don't need to setup the pipeline to compute it
    /*
    Atmosphere.LutVertexShader = D3D11_LoadVertexShader(Device, L"../src/shaders/bruneton/atmosphere_lut.hlsl", 0);
    Atmosphere.LutGeometryShader = D3D11_LoadGeometryShader(Device, L"../src/shaders/bruneton/atmosphere_lut.hlsl");
    Atmosphere.TransmittanceShader = D3D11_LoadPixelShader(Device, L"../src/shaders/bruneton/atmosphere_transmittance.hlsl");
    Atmosphere.DirectIrradianceShader = D3D11_LoadPixelShader(Device, L"../src/shaders/bruneton/atmosphere_direct_irradiance.hlsl");
    Atmosphere.SingleScatterShader = D3D11_LoadPixelShader(Device, L"../src/shaders/bruneton/atmosphere_single_scatter.hlsl");
    Atmosphere.ScatteringDensityShader = D3D11_LoadPixelShader(Device, L"../src/shaders/bruneton/atmosphere_scattering_density.hlsl");
    Atmosphere.IndirectIrradianceShader = D3D11_LoadPixelShader(Device, L"../src/shaders/bruneton/atmosphere_indirect_irradiance.hlsl");
    Atmosphere.MultipleScatteringShader = D3D11_LoadPixelShader(Device, L"../src/shaders/bruneton/atmosphere_multiple_scattering.hlsl");
    
    Atmosphere.ScatteringOrderBuffer = D3D11_CreateConstantBuffer(Device, sizeof(s32));
    
    Atmosphere.DeltaIrradiance = D3D11_CreateRenderTarget(Device, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT,
                                                          ATMOSPHERE_LUT_FORMAT);
    s32 x = SCATTERING_TEXTURE_WIDTH;
    s32 y = SCATTERING_TEXTURE_HEIGHT;
    s32 z = SCATTERING_TEXTURE_DEPTH;
    Atmosphere.DeltaScatteringR = D3D11_CreateRenderTarget3D(Device, x, y, z, ATMOSPHERE_LUT_FORMAT);
    Atmosphere.DeltaScatteringM = D3D11_CreateRenderTarget3D(Device, x, y, z, ATMOSPHERE_LUT_FORMAT);
    Atmosphere.DeltaScatteringDensity = D3D11_CreateRenderTarget3D(Device, x, y, z, ATMOSPHERE_LUT_FORMAT);
    */
    
    D3D11_BLEND_DESC BlendDesc = {};
	BlendDesc.IndependentBlendEnable = true;
    //No blending: result = src
	BlendDesc.RenderTarget[0].BlendEnable = false;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    //Additive blending: result = src + dest
	BlendDesc.RenderTarget[1].BlendEnable = true;
	BlendDesc.RenderTarget[1].SrcBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[1].DestBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[1].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    Device->CreateBlendState(&BlendDesc, &Atmosphere.Blend0Nop1Add);
    
    //color: result = src + (1 - src_alpha) * dest;
    //alpha: result = dest_alpha
    BlendDesc = {};
    BlendDesc.AlphaToCoverageEnable = false;
    BlendDesc.IndependentBlendEnable = false;
    BlendDesc.RenderTarget[0].BlendEnable = true;
    BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    
    Device->CreateBlendState(&BlendDesc, &Atmosphere.RenderBlend);
    
    Atmosphere.RenderAtmosphereShader = D3D11_LoadPixelShader(Device, L"../src/shaders/bruneton/atmosphere_render.hlsl");
    Atmosphere.RenderAtmosphereBuffer = D3D11_CreateConstantBuffer(Device, sizeof(d3d11_atmosphere_render_constants));
    
    return Atmosphere;
}

internal d3d11_shadow_pipeline
D3D11_InitShadowPipeline(d3d11_state* D3D11)
{
    d3d11_shadow_pipeline Shadow = {};
    
    ID3D11Device* Device = D3D11->Device;
    HRESULT Result;
    
    D3D11_RASTERIZER_DESC RasterDesc = {};
    RasterDesc.FillMode = D3D11_FILL_SOLID;
    RasterDesc.DepthClipEnable = true;
    RasterDesc.FrontCounterClockwise = MESH_WINDING_COUNTER_CLOCKWISE;
    RasterDesc.CullMode = D3D11_CULL_FRONT;
    Result = Device->CreateRasterizerState(&RasterDesc, &Shadow.RasterState);
    Assert(Result == S_OK);
    
    ID3D10Blob* Buffer = 0;
    Shadow.VertexShader = D3D11_LoadVertexShader(Device, L"../src/shaders/shadow_map.hlsl", &Buffer);
    Buffer->Release();
    
    Shadow.CubemapVertexShader = D3D11_LoadVertexShader(Device, L"../src/shaders/shadow_cubemap.hlsl", &Buffer);
    Shadow.CubemapPixelShader = D3D11_LoadPixelShader(Device, L"../src/shaders/shadow_cubemap.hlsl");
    
    D3D11_INPUT_ELEMENT_DESC LayoutDesc[1] = {};
    LayoutDesc[0].SemanticName = "POSITION";
    LayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    LayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    Result = D3D11->Device->CreateInputLayout(LayoutDesc, ArrayCount(LayoutDesc),
                                              Buffer->GetBufferPointer(),
                                              Buffer->GetBufferSize(), &Shadow.Layout);
    Buffer->Release();
    
    Assert(Result == S_OK);
    
    Shadow.VertexConstantsBuffer = D3D11_CreateConstantBuffer(Device, sizeof(d3d11_shadow_vertex_constants));
    Shadow.PixelConstantsBuffer = D3D11_CreateConstantBuffer(Device, sizeof(d3d11_shadow_pixel_constants));
    
    return Shadow;
}

internal d3d11_sh_pipeline
D3D11_InitSHPipeline(d3d11_state* D3D11)
{
    ID3D11Device* Device = D3D11->Device;
    
    ID3D10Blob* Buffer = 0;
    
    d3d11_sh_pipeline SH = {};
    SH.TestVertexShader = D3D11_LoadVertexShader(Device, L"../src/shaders/sh_test.hlsl", &Buffer);
    SH.TestPixelShader = D3D11_LoadPixelShader(Device, L"../src/shaders/sh_test.hlsl");
    Buffer->Release();
    
    SH.LightVertexShader = D3D11_LoadVertexShader(Device, L"../src/shaders/sh_light_test.hlsl", &Buffer);
    SH.LightPixelShader = D3D11_LoadPixelShader(Device, L"../src/shaders/sh_light_test.hlsl");
    Buffer->Release();
    
    SH.PixelConstantsBuffer = D3D11_CreateConstantBuffer(Device, sizeof(vec4) * 9);
    
    return SH;
}


internal d3d11_debug_pipeline
D3D11_InitDebugPipeline(d3d11_state* D3D11)
{
    d3d11_debug_pipeline DebugPipeline = {};
    
    ID3D11Device* Device = D3D11->Device;
    HRESULT Result;
    
    ID3D10Blob* Buffer = 0;
    DebugPipeline.ColorVertex = D3D11_LoadVertexShader(Device,  L"../src/shaders/color.hlsl", &Buffer);
    DebugPipeline.ColorPixel = D3D11_LoadPixelShader(Device,  L"../src/shaders/color.hlsl");
    DebugPipeline.VertexConstantsBuffer = D3D11_CreateConstantBuffer(Device, sizeof(mat4) * 2);
    
    D3D11_INPUT_ELEMENT_DESC LayoutDesc[2] = {};
    LayoutDesc[0].SemanticName = "POSITION";
    LayoutDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    LayoutDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    
    LayoutDesc[1].SemanticName = "COLOR";
    LayoutDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    LayoutDesc[1].AlignedByteOffset = offsetof(color_vertex, Color);
    LayoutDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    Result = D3D11->Device->CreateInputLayout(LayoutDesc, ArrayCount(LayoutDesc),
                                              Buffer->GetBufferPointer(),
                                              Buffer->GetBufferSize(), &DebugPipeline.ColorLayout);
    Assert(Result == S_OK);
    
    DebugPipeline.LineVertices = D3D11_CreateMappableBuffer(Device, sizeof(color_vertex) * 2 * MAX_DEBUG_LINES);
    DebugPipeline.TriangleVertices = D3D11_CreateMappableBuffer(Device, sizeof(color_vertex) * 3 * MAX_DEBUG_TRIANGLES);
    
    return DebugPipeline;
}



internal d3d11_state
D3D11_Init(HWND Window, s32 BufferWidth, s32 BufferHeight)
{
    HRESULT Result;
    
    IDXGIFactory* Factory = 0;
    Result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&Factory));
    Assert(Result == S_OK);
    
    IDXGIAdapter* Adapter = 0;
    DXGI_ADAPTER_DESC AdapterDesc = {};
    // TODO: comparison of enumaration results
    Factory->EnumAdapters(0, &Adapter);
    Adapter->GetDesc(&AdapterDesc);
    //wprintf(L"Adapter: %s\n", AdapterDesc.Description);
    
    IDXGIOutput* Output = 0;
    // TODO: comparison of enumaration results
    Adapter->EnumOutputs(0, &Output);
    DXGI_OUTPUT_DESC OutputDesc = {};
    Output->GetDesc(&OutputDesc);
    
    s32 MonitorWidth = OutputDesc.DesktopCoordinates.right - OutputDesc.DesktopCoordinates.left;
    s32 MonitorHeight = OutputDesc.DesktopCoordinates.bottom - OutputDesc.DesktopCoordinates.top;
    
    DXGI_MODE_DESC DisplayModeDesc = {};
    DXGI_MODE_DESC MatchDesc = {};
    MatchDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    Output->FindClosestMatchingMode(&MatchDesc, &DisplayModeDesc, NULL);
    f32 MonitorRefreshRate = (f32)DisplayModeDesc.RefreshRate.Numerator / (f32)DisplayModeDesc.RefreshRate.Denominator;
    
    //Device and Context
    ID3D11Device* Device = 0;
    D3D_FEATURE_LEVEL FeatureLevel[1] = {
        D3D_FEATURE_LEVEL_11_1
    };
    ID3D11DeviceContext* DeviceContext = 0;
    UINT CreationFlags = D3D11_CREATE_DEVICE_DEBUG;
    Result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, CreationFlags, FeatureLevel, 1,
                               D3D11_SDK_VERSION, &Device, 0, &DeviceContext);
    Assert(Result == S_OK);
    
    //SwapChain
    DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.BufferDesc.Width = BufferWidth;
    SwapChainDesc.BufferDesc.Height = BufferHeight;
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc.SampleDesc.Count = 1;
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDesc.OutputWindow = Window;
    SwapChainDesc.Windowed = true;
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    
    IDXGISwapChain* SwapChain = 0;
    Result = Factory->CreateSwapChain(Device, &SwapChainDesc, &SwapChain);
    Assert(Result == S_OK);
    
    STARTUP_TIMESTAMP(D3D11_DEVICE);
    
    //Render target
    ID3D11Texture2D* BackBuffer = 0;
    Result = SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);
    Assert(Result == S_OK);
    D3D11_TEXTURE2D_DESC BackBufferDesc = {};
    BackBuffer->GetDesc(&BackBufferDesc);
    
    ID3D11RenderTargetView* BackbufferRenderTarget = 0;
    Result = Device->CreateRenderTargetView(BackBuffer, 0, &BackbufferRenderTarget);
    Assert(Result == S_OK);
    BackBuffer->Release();
    
    //Default render target
    d3d11_render_target DefaultRenderTarget = D3D11_CreateRenderTargetLDR(Device, BufferWidth, BufferHeight);
    
    //Default Depth stencil
    D3D11_TEXTURE2D_DESC DepthStencilBufferDesc = {};
    DepthStencilBufferDesc.Width = BackBufferDesc.Width;
    DepthStencilBufferDesc.Height = BackBufferDesc.Height;
    DepthStencilBufferDesc.MipLevels = 1;
    DepthStencilBufferDesc.ArraySize = 1;
    DepthStencilBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    DepthStencilBufferDesc.SampleDesc.Count = 1;
    DepthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    DepthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    
    ID3D11Texture2D* DepthStencilBuffer = {};
    Result = Device->CreateTexture2D(&DepthStencilBufferDesc, NULL, &DepthStencilBuffer);
    Assert(Result == S_OK);
    
    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
    DepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    
    ID3D11DepthStencilView* DepthStencil = 0;
    Result = Device->CreateDepthStencilView(DepthStencilBuffer, &DepthStencilViewDesc, &DepthStencil);
    Assert(Result == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC DepthStencilResourceDesc = {};
    DepthStencilResourceDesc.Format = DXGI_FORMAT_R32_FLOAT;
    DepthStencilResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    DepthStencilResourceDesc.Texture2D.MipLevels = 1;
    
    ID3D11ShaderResourceView* DepthStencilResourceView;
    Result = Device->CreateShaderResourceView(DepthStencilBuffer, &DepthStencilResourceDesc, &DepthStencilResourceView);
    Assert(Result == S_OK);
    
    STARTUP_TIMESTAMP(D3D11_DEFAULT_TARGETS);
    
    //Viewport
    D3D11_VIEWPORT Viewport = {};
    Viewport.Width = (float)BackBufferDesc.Width;
    Viewport.Height = (float)BackBufferDesc.Height;
    Viewport.MaxDepth = 1.0f;
    
    //Return
    d3d11_state D3D11 = {};
    D3D11.Device = Device;
    D3D11.Context = DeviceContext;
    D3D11.SwapChain = SwapChain;
    D3D11.MonitorInfo.Width = MonitorWidth;
    D3D11.MonitorInfo.Height = MonitorHeight;
    D3D11.MonitorInfo.RefreshRate = MonitorRefreshRate;
    D3D11.Viewport = Viewport;
    
    D3D11.BackbufferRenderTargetView = BackbufferRenderTarget;
    
    D3D11.DefaultRenderTarget = DefaultRenderTarget;
    D3D11.DefaultDepthStencilView = DepthStencil;
    D3D11.DefaultDepthStencilBuffer = DepthStencilBuffer;
    D3D11.DefaultDepthStencilResourceView = DepthStencilResourceView;
    
    D3D11.Common = D3D11_InitCommonState(Device); 
    
    STARTUP_TIMESTAMP(D3D11_COMMON);
    
    // IMPORTANT: Init pipelines after Common state has been inited
    
    D3D11.PBR = D3D11_InitPBRPipeline(&D3D11, BufferWidth, BufferHeight);
    STARTUP_TIMESTAMP(D3D11_PBR);    
    
    //D3D11.Precompute = D3D11_InitPrecomputePipeline(&D3D11);
    STARTUP_TIMESTAMP(D3D11_PRECOMPUTE_LIGHT_PROBES);
    
    D3D11.Shadow = D3D11_InitShadowPipeline(&D3D11);
    STARTUP_TIMESTAMP(D3D11_SHADOW_MAPS);
    
    D3D11.Atmosphere = D3D11_InitAtmospherePipeline(&D3D11);
    STARTUP_TIMESTAMP(D3D11_PRECOMPUTE_ATMOSPHERE);
    
    D3D11.SH = D3D11_InitSHPipeline(&D3D11);
    STARTUP_TIMESTAMP(D3D11_SPHERICAL_HARMONICS);
                                        
    D3D11.Debug = D3D11_InitDebugPipeline(&D3D11);
    STARTUP_TIMESTAMP(D3D11_DEBUG);
    
    D3D11.Profiler = D3D11_InitProfiler(Device);
    STARTUP_TIMESTAMP(D3D11_PROFILER);
    
    return D3D11;
}

internal void
D3D11_Clear(d3d11_state* D3D11, vec4 ClearColor)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    
    // Clear the back buffer.
    Context->ClearRenderTargetView(D3D11->DefaultRenderTarget.RenderTarget, ClearColor.e);
#if NEAR_FAR_PLANE_INVERSION
    f32 DepthValue = 0.0f;
#else
    f32 DepthValue = 1.0f;
#endif
    Context->ClearDepthStencilView(D3D11->DefaultDepthStencilView, D3D11_CLEAR_DEPTH, DepthValue, 0);
}

internal void
D3D11_ResizeRenderTarget(ID3D11Device* Device, d3d11_render_target* Target, u32 Width, u32 Height)
{
    Target->Width = Width;
    Target->Height = Height;
    
    D3D11_TEXTURE2D_DESC TextureDesc;
    Target->Texture->GetDesc(&TextureDesc);
    TextureDesc.Width = Width;
    TextureDesc.Height = Height;
    
    D3D11_SHADER_RESOURCE_VIEW_DESC ResourceDesc;
    Target->ResourceView->GetDesc(&ResourceDesc);
    
    D3D11_RENDER_TARGET_VIEW_DESC RenderTargetDesc;
    Target->RenderTarget->GetDesc(&RenderTargetDesc);
    
    Target->ResourceView->Release();
    Target->Texture->Release();
    Target->RenderTarget->Release();
    
    HRESULT HResult = Device->CreateTexture2D(&TextureDesc, 0, &Target->Texture);
    Assert(HResult == S_OK);
    HResult = Device->CreateShaderResourceView(Target->Texture, &ResourceDesc, &Target->ResourceView);
    Assert(HResult == S_OK);
    HResult = Device->CreateRenderTargetView(Target->Texture, &RenderTargetDesc, &Target->RenderTarget);
    Assert(HResult == S_OK);
}

internal void
D3D11_ResizeDefaultRenderTargets(d3d11_state* D3D11, ivec2 Size)
{
    ID3D11Device* Device = D3D11->Device;
    
    s32 Width = Size.x;
    s32 Height = Size.y;
    
    //Default render target resize
    D3D11_ResizeRenderTarget(Device, &D3D11->DefaultRenderTarget, Width, Height);
    
    //Default depth stencil resize
    D3D11_TEXTURE2D_DESC DepthTextureDesc;
    D3D11->DefaultDepthStencilBuffer->GetDesc(&DepthTextureDesc);
    DepthTextureDesc.Width = Width;
    DepthTextureDesc.Height = Height;
    D3D11->DefaultDepthStencilBuffer->Release();
    
    D3D11_DEPTH_STENCIL_VIEW_DESC DepthViewDesc;
    D3D11->DefaultDepthStencilView->GetDesc(&DepthViewDesc);
    D3D11->DefaultDepthStencilView->Release();
    
    D3D11_SHADER_RESOURCE_VIEW_DESC DepthResourceViewDesc;
    D3D11->DefaultDepthStencilResourceView->GetDesc(&DepthResourceViewDesc);
    D3D11->DefaultDepthStencilResourceView->Release();
    
    HRESULT Result = Device->CreateTexture2D(&DepthTextureDesc, NULL, &D3D11->DefaultDepthStencilBuffer);
    Assert(Result == S_OK);
    Result = Device->CreateDepthStencilView(D3D11->DefaultDepthStencilBuffer, &DepthViewDesc, &D3D11->DefaultDepthStencilView);
    Assert(Result == S_OK);
    Result = Device->CreateShaderResourceView(D3D11->DefaultDepthStencilBuffer, &DepthResourceViewDesc, &D3D11->DefaultDepthStencilResourceView);
    Assert(Result == S_OK);
    
    //Resize other render targets
    D3D11_ResizeRenderTarget(Device, &D3D11->PBR.IntermediateTarget, Width, Height);
    
    //Resize viewport
    D3D11->Viewport.Width = (float)Width;
    D3D11->Viewport.Height = (float)Height;    
}

internal void
D3D11_ResizeBackbuffer(d3d11_state* D3D11, ivec2 Size)
{
    u32 Width = Size.x;
    u32 Height = Size.y;
    
    D3D11->Context->OMSetRenderTargets(0, 0, 0);
    
    //Backbuffer render target resize
    if(D3D11->BackbufferRenderTargetView)
    {
        D3D11->BackbufferRenderTargetView->Release();
        D3D11->BackbufferRenderTargetView = NULL;
    }
    
    HRESULT Result = D3D11->SwapChain->ResizeBuffers(0, Width, Height, DXGI_FORMAT_UNKNOWN, 0);
    Assert(Result == S_OK);
    
    ID3D11Device* Device = D3D11->Device;
    ID3D11Texture2D* BackBuffer;
    Result = D3D11->SwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
    
    Assert(Result == S_OK);
    Result = Device->CreateRenderTargetView(BackBuffer, NULL, &D3D11->BackbufferRenderTargetView);
    Assert(Result == S_OK);
    BackBuffer->Release();
    
    //Resize viewport
    D3D11->BackbufferViewport.Width = (float)Width;
    D3D11->BackbufferViewport.Height = (float)Height;    
}

internal void
D3D11_Present(d3d11_state* D3D11, bool VSyncEnabled)
{
    D3D11->SwapChain->Present(VSyncEnabled ? 1 : 0, 0);
}

internal d3d11_mesh
D3D11_LoadMesh(ID3D11Device* Device, mesh_data* Mesh)
{
    d3d11_mesh Result = {};
    Result.VertexBuffers[0] = D3D11_CreateBuffer(Device, Mesh->Positions, sizeof(vec3) * Mesh->VerticesCount);
    Result.VertexBuffers[1] = D3D11_CreateBuffer(Device, Mesh->Normals,   sizeof(vec3) * Mesh->VerticesCount);
    Result.VertexBuffers[2] = D3D11_CreateBuffer(Device, Mesh->Tangents,  sizeof(vec3) * Mesh->VerticesCount);
    Result.VertexBuffers[3] = D3D11_CreateBuffer(Device, Mesh->UVs,       sizeof(vec2) * Mesh->VerticesCount);
    
    if(Mesh->Flags & MESH_HAS_ANIMATION)
    {
        Result.VertexBuffers[4] = D3D11_CreateBuffer(Device, Mesh->Weights, sizeof(vec4)  * Mesh->VerticesCount);
        Result.VertexBuffers[5] = D3D11_CreateBuffer(Device, Mesh->Joints,  sizeof(ivec4) * Mesh->VerticesCount);
    }
    
    Result.VerticesCount = Mesh->VerticesCount;
    
    if(!(Mesh->Flags & MESH_NO_INDICES))
    {
        Result.IndexBuffer = D3D11_CreateBuffer(Device, Mesh->Indices, sizeof(u32) * Mesh->IndicesCount, true);
        Result.IndicesCount = Mesh->IndicesCount;
    }
    Result.Flags = Mesh->Flags;
    
    return Result;
}

internal d3d11_texture
D3D11_CreateTexture(ID3D11Device* Device, image_data* Image, DXGI_FORMAT Format)
{
    d3d11_texture Result = {};
    
    HRESULT HResult;
    D3D11_SUBRESOURCE_DATA TextureData = {};
    TextureData.pSysMem = Image->Data;
    TextureData.SysMemPitch = Image->Pitch;
    
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = Image->Width;
    TextureDesc.Height = Image->Height;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = Format;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
    HResult = Device->CreateTexture2D(&TextureDesc, &TextureData, &Result.Texture);
    Assert(HResult == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC TextureResourceDesc = {};
    TextureResourceDesc.Format = Format;
    TextureResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    TextureResourceDesc.Texture2D.MipLevels = (u32)-1; //Use all mips
    
    HResult = Device->CreateShaderResourceView(Result.Texture, &TextureResourceDesc, &Result.ResourceView);
    Assert(HResult == S_OK);
    
    return Result;
}

internal d3d11_texture_3d
D3D11_CreateTexture3D(ID3D11Device* Device, image_3d_data* Image, DXGI_FORMAT Format)
{
    d3d11_texture_3d Result = {};
    
    HRESULT HResult;
    D3D11_SUBRESOURCE_DATA TextureData = {};
    TextureData.pSysMem = Image->Data;
    TextureData.SysMemPitch = Image->Width * Image->BytesPerPixel;
    TextureData.SysMemSlicePitch = Image->Height * TextureData.SysMemPitch;
        
    D3D11_TEXTURE3D_DESC TextureDesc = {};
    TextureDesc.Width = Image->Width;
    TextureDesc.Height = Image->Height;
    TextureDesc.Depth = Image->Depth;
    TextureDesc.MipLevels = 1;
    TextureDesc.Format = Format;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
    HResult = Device->CreateTexture3D(&TextureDesc, &TextureData, &Result.Texture);
    Assert(HResult == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC TextureResourceDesc = {};
    TextureResourceDesc.Format = Format;
    TextureResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    TextureResourceDesc.Texture3D.MipLevels = (u32)-1; //Use all mips
    
    HResult = Device->CreateShaderResourceView(Result.Texture, &TextureResourceDesc, &Result.ResourceView);
    Assert(HResult == S_OK);
    
    return Result;
}

internal d3d11_shadow_map
D3D11_CreateShadowMap(ID3D11Device* Device, u32 Width, u32 Height)
{
    d3d11_shadow_map Result = {};
    Result.Width = Width;
    Result.Height = Height;
    
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = Width;
    TextureDesc.Height = Height;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    
    HRESULT HResult = Device->CreateTexture2D(&TextureDesc, NULL, &Result.Texture);
    Assert(HResult == S_OK);
    
    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
    DepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    
    HResult = Device->CreateDepthStencilView(Result.Texture, &DepthStencilViewDesc, &Result.DepthView);
    Assert(HResult == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC ResourceDesc = {};
    ResourceDesc.Format = DXGI_FORMAT_R32_FLOAT;
    ResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ResourceDesc.Texture2D.MipLevels = 1;
    
    HResult = Device->CreateShaderResourceView(Result.Texture, &ResourceDesc, &Result.ResourceView);
    Assert(HResult == S_OK);
    
    return Result;
}

internal d3d11_shadow_cubemap
D3D11_CreateShadowCubemap(ID3D11Device* Device, u32 Size)
{
    d3d11_shadow_cubemap Result = {};
    Result.Size = Size;
    
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = Size;
    TextureDesc.Height = Size;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 6;
    TextureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    TextureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    
    HRESULT HResult = Device->CreateTexture2D(&TextureDesc, NULL, &Result.Texture);
    Assert(HResult == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC ResourceDesc = {};
    ResourceDesc.Format = DXGI_FORMAT_R32_FLOAT;
    ResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    ResourceDesc.Texture2D.MipLevels = 1;
    
    HResult = Device->CreateShaderResourceView(Result.Texture, &ResourceDesc, &Result.ResourceView);
    Assert(HResult == S_OK);
    
    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc = {};
    DepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    DepthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    DepthStencilViewDesc.Texture2DArray.ArraySize = 1;
    
    
    for(u32 Index = 0; Index < 6; Index++)
    {
        DepthStencilViewDesc.Texture2DArray.FirstArraySlice = Index;
        HResult = Device->CreateDepthStencilView(Result.Texture, &DepthStencilViewDesc, &Result.DepthViews[Index]);
        Assert(HResult == S_OK);
    }
    
    return Result;
}

internal d3d11_depth_buffer
D3D11_CreateDepthBuffer(ID3D11Device* Device, u32 Width, u32 Height)
{
    d3d11_depth_buffer Result = {};
    
    //Depth stencil
    D3D11_TEXTURE2D_DESC BufferDesc = {};
    BufferDesc.Width = Width;
    BufferDesc.Height = Height;
    BufferDesc.MipLevels = 1;
    BufferDesc.ArraySize = 1;
    BufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
    BufferDesc.SampleDesc.Count = 1;
    BufferDesc.Usage = D3D11_USAGE_DEFAULT;
    BufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    
    HRESULT HResult = Device->CreateTexture2D(&BufferDesc, NULL, &Result.Texture);
    Assert(HResult == S_OK);
    
    D3D11_DEPTH_STENCIL_VIEW_DESC ViewDesc = {};
    ViewDesc.Format = BufferDesc.Format;
    ViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    HResult = Device->CreateDepthStencilView(Result.Texture, &ViewDesc, &Result.DepthView);
    Assert(HResult == S_OK);
    
    return Result;
}

internal void
D3D11_FreeDepthBuffer(d3d11_depth_buffer* Depth)
{
    Depth->DepthView->Release();
    Depth->Texture->Release();
}

internal d3d11_texture
D3D11_CreateHDRTexture(ID3D11Device* Device, image_data* Image)
{
    HRESULT HResult;
    D3D11_SUBRESOURCE_DATA TextureData = {};
    TextureData.pSysMem = Image->Data;
    TextureData.SysMemPitch = Image->Pitch;
    
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width = Image->Width;
    TextureDesc.Height = Image->Height;
    TextureDesc.MipLevels = 1;
    TextureDesc.ArraySize = 1;
    TextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    TextureDesc.SampleDesc.Count = 1;
    TextureDesc.Usage = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
    ID3D11Texture2D* Texture = 0;
    HResult = Device->CreateTexture2D(&TextureDesc, &TextureData, &Texture);
    Assert(HResult == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC TextureResourceDesc = {};
    TextureResourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    TextureResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    TextureResourceDesc.Texture2D.MipLevels = (u32)-1;
    
    ID3D11ShaderResourceView* TextureResource = 0;
    HResult = Device->CreateShaderResourceView(Texture, &TextureResourceDesc, &TextureResource);
    Assert(HResult == S_OK);
    
    d3d11_texture Result = {};
    Result.ResourceView = TextureResource;
    Result.Texture = Texture;
    
    return Result;
}

internal d3d11_cubemap_target
D3D11_CreateCubeTextureAndRenderTarget(ID3D11Device* Device, u32 Size, b32 Mips = false)
{
    d3d11_cubemap_target Result = {};
    
    D3D11_TEXTURE2D_DESC CubemapDesc = {};
    CubemapDesc.Width = Size;
    CubemapDesc.Height = Size;
    CubemapDesc.MipLevels = Mips ? 0 : 1;
    CubemapDesc.ArraySize = 6;
    CubemapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    CubemapDesc.SampleDesc.Count = 1;
    CubemapDesc.Usage = D3D11_USAGE_DEFAULT;
    CubemapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    CubemapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
    
    HRESULT HResult = Device->CreateTexture2D(&CubemapDesc, 0, &Result.Texture);
    Assert(HResult == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC CubemapResourceDesc = {};
    CubemapResourceDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    CubemapResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    CubemapResourceDesc.TextureCube.MipLevels = (u32)-1;
    
    HResult = Device->CreateShaderResourceView(Result.Texture, &CubemapResourceDesc, &Result.ResourceView);
    Assert(HResult == S_OK);
    
    D3D11_RENDER_TARGET_VIEW_DESC CubemapRenderTargetDesc = {};
    CubemapRenderTargetDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    CubemapRenderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    CubemapRenderTargetDesc.Texture2DArray.ArraySize = 1;
    
    for(u32 Index = 0; Index < 6; Index++)
    {
        CubemapRenderTargetDesc.Texture2DArray.FirstArraySlice = Index;
        HResult = Device->CreateRenderTargetView(Result.Texture, &CubemapRenderTargetDesc, &Result.RenderTargets[Index]);
        Assert(HResult == S_OK);
    }
    
    return Result;
}


internal d3d11_cubemap_target_mips
D3D11_CreateCubeTextureAndRenderTargetMips(ID3D11Device* Device, u32 Size)
{
    d3d11_cubemap_target_mips Result = {};
    u32 NumberOfMips = ArrayCount(Result.RenderTargets[0]);
        
    D3D11_TEXTURE2D_DESC CubemapDesc = {};
    CubemapDesc.Width = Size;
    CubemapDesc.Height = Size;
    CubemapDesc.MipLevels = NumberOfMips;
    CubemapDesc.ArraySize = 6;
    CubemapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    CubemapDesc.SampleDesc.Count = 1;
    CubemapDesc.Usage = D3D11_USAGE_DEFAULT;
    CubemapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    CubemapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    
    HRESULT HResult = Device->CreateTexture2D(&CubemapDesc, 0, &Result.Texture);
    Assert(HResult == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC CubemapResourceDesc = {};
    CubemapResourceDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    CubemapResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    CubemapResourceDesc.TextureCube.MipLevels = (u32)-1;
    
    HResult = Device->CreateShaderResourceView(Result.Texture, &CubemapResourceDesc, &Result.ResourceView);
    Assert(HResult == S_OK);
    
    D3D11_RENDER_TARGET_VIEW_DESC CubemapRenderTargetDesc = {};
    CubemapRenderTargetDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    CubemapRenderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
    CubemapRenderTargetDesc.Texture2DArray.ArraySize = 1;
    
    for(u32 ArrayIndex = 0; ArrayIndex < 6; ArrayIndex++)
    {
        CubemapRenderTargetDesc.Texture2DArray.FirstArraySlice = ArrayIndex;
        for(u32 MipIndex = 0; MipIndex < 5; MipIndex++)
        {
            CubemapRenderTargetDesc.Texture2DArray.MipSlice = MipIndex;
            HResult = Device->CreateRenderTargetView(Result.Texture, &CubemapRenderTargetDesc, &Result.RenderTargets[ArrayIndex][MipIndex]);
            Assert(HResult == S_OK);
        }
    }
    
    return Result;
}


internal d3d11_cubemap
D3D11_LoadCubemap(ID3D11Device* Device, cubemap_data* CubemapData)
{
    s32 Size = CubemapData->Size;
    u32 NumberOfMips = CubemapData->NumberOfMips;
    
    
    D3D11_TEXTURE2D_DESC CubemapDesc = {};
    CubemapDesc.Width = Size;
    CubemapDesc.Height = Size;
    CubemapDesc.MipLevels = NumberOfMips;
    CubemapDesc.ArraySize = 6;
    CubemapDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    CubemapDesc.SampleDesc.Count = 1;
    CubemapDesc.Usage = D3D11_USAGE_DEFAULT;
    CubemapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    CubemapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    
    Assert(NumberOfMips <= 10 && NumberOfMips > 0);
    D3D11_SUBRESOURCE_DATA ResourceDataArray[6 * 10]; //One for each face and mip
    
    u8* CurrentData = CubemapData->Data;
    
    for(u32 FaceIndex = 0; FaceIndex < 6; FaceIndex++)
    {
        u32 MipSize = Size;
        for(u32 MipIndex = 0; MipIndex < CubemapData->NumberOfMips; MipIndex++)
        {
            D3D11_SUBRESOURCE_DATA* ResourceData = ResourceDataArray + (FaceIndex * NumberOfMips) + MipIndex;
            
            ResourceData->pSysMem = CurrentData;
            u32 MipPitch = MipSize * CubemapData->BytesPerPixel;
            CurrentData += MipSize * MipPitch;
            ResourceData->SysMemPitch = MipPitch;
            
            MipSize /= 2;
        }
    }
    
    ID3D11Texture2D* Texture = 0;
    HRESULT HResult = Device->CreateTexture2D(&CubemapDesc, ResourceDataArray, &Texture);
    Assert(HResult == S_OK);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC CubemapResourceDesc = {};
    CubemapResourceDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    CubemapResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    CubemapResourceDesc.TextureCube.MipLevels = (u32)-1;
    
    ID3D11ShaderResourceView* ResourceView;
    HResult = Device->CreateShaderResourceView(Texture, &CubemapResourceDesc, &ResourceView);
    Assert(HResult == S_OK);
    
    d3d11_cubemap Result = {};
    Result.Texture = Texture;
    Result.ResourceView = ResourceView;
    
    return Result;
}

internal d3d11_light_probe
D3D11_PrecomputeLightProbeFromHDR(d3d11_state* D3D11, char* PathToHDRImage)
{
    ID3D11Device* Device = D3D11->Device;
    ID3D11DeviceContext* Context = D3D11->Context;
    
    //Capture data
    mat4 CaptureProj = Mat4Perspective(90.0f, 0.1f, 10.0f, 1.0f);
    mat4 CaptureViews[] = {
        Mat4LookAt(vec3(0.0f), vec3(-1.0f,  0.0f,  0.0f), vec3(0.0f,  1.0f,  0.0f)), //1 -x
        Mat4LookAt(vec3(0.0f), vec3( 1.0f,  0.0f,  0.0f), vec3(0.0f,  1.0f,  0.0f)), //0 +x
        Mat4LookAt(vec3(0.0f), vec3( 0.0f,  1.0f,  0.0f), vec3(0.0f,  0.0f, -1.0f)), //2 +y
        Mat4LookAt(vec3(0.0f), vec3( 0.0f, -1.0f,  0.0f), vec3(0.0f,  0.0f,  1.0f)), //3 -y
        Mat4LookAt(vec3(0.0f), vec3( 0.0f,  0.0f,  1.0f), vec3(0.0f,  1.0f,  0.0f)), //4 +z
        Mat4LookAt(vec3(0.0f), vec3( 0.0f,  0.0f, -1.0f), vec3(0.0f,  1.0f,  0.0f)), //5 -z
    };
    
    d3d11_precompute_pixel_constants PixelConstants = {};
    d3d11_precompute_vertex_constants VertexConstants = {};
    
    ID3D11Buffer* VertexConstantsBuffer = D3D11->Precompute.VertexConstantsBuffer;
    ID3D11Buffer* PixelConstantsBuffer = D3D11->Precompute.PixelConstantsBuffer;
    
    //HDR radiance cubemap
    image_data HDRImage = LoadHDRImageFromFile(PathToHDRImage);
    d3d11_texture HDRTexture = D3D11_CreateHDRTexture(Device, &HDRImage);
    FreeImage(&HDRImage);
    
    u32 CubemapSize = HDR_CUBEMAP_SIZE;
    d3d11_cubemap_target Cubemap = D3D11_CreateCubeTextureAndRenderTarget(Device, CubemapSize, true);
    D3D11_VIEWPORT CaptureViewport = {};
    CaptureViewport.Width = (float)CubemapSize;
    CaptureViewport.Height = (float)CubemapSize;
    CaptureViewport.MaxDepth = 1.0f;
    
    Context->ClearState();
    Context->OMSetDepthStencilState(D3D11->Common.LessDepthStencilState, 0);
    Context->RSSetState(D3D11->Common.SolidRasterStateNoCull);
    Context->RSSetViewports(1, &CaptureViewport);
    {
        u32 Strides[] = {sizeof(vec3)};
        u32 Offsets[] = {0};
        Context->IASetVertexBuffers(0, 1, &D3D11->Common.CubemapVertexBuffer, Strides, Offsets);
    }
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(D3D11->Precompute.Layout);
    Context->VSSetShader(D3D11->Precompute.VertexShader, 0, 0);
    Context->PSSetShader(D3D11->Precompute.EquirectPixelShader, 0, 0);
    Context->PSSetShaderResources(0, 1, &HDRTexture.ResourceView);
    Context->PSSetSamplers(0, 1, &D3D11->Common.LinearSamplerState);
    
    for(u32 Index = 0; Index < 6; Index++)
    {
        ID3D11RenderTargetView* RenderTarget = Cubemap.RenderTargets[Index];
        
        Context->ClearRenderTargetView(RenderTarget, vec4(1.0f).e);
        Context->OMSetRenderTargets(1, &RenderTarget, 0);
        
        //Vertex constants
        VertexConstants.Projection = CaptureProj;
        VertexConstants.View = CaptureViews[Index];
        
        D3D11_FillConstantBuffers(Context, VertexConstantsBuffer, &VertexConstants, sizeof(VertexConstants));
        Context->VSSetConstantBuffers(0, 1, &VertexConstantsBuffer);
        
        Context->Draw(36, 0);
        
        //Free the render target view, we only need to draw to each texture once
        Context->OMSetRenderTargets(0, 0, 0);
        RenderTarget->Release();
    }
    
    
    //Irradiance Cubemap
    u32 IrradianceSize = IRRADIANCE_CUBEMAP_SIZE;
    d3d11_cubemap_target IrradianceCube = D3D11_CreateCubeTextureAndRenderTarget(Device, IrradianceSize);
    D3D11_VIEWPORT IrradianceViewport = {};
    IrradianceViewport.Width = (float)IrradianceSize;
    IrradianceViewport.Height = (float)IrradianceSize;
    IrradianceViewport.MaxDepth = 1.0f;
    
    Context->ClearState();
    Context->OMSetDepthStencilState(D3D11->Common.LessDepthStencilState, 0);
    Context->RSSetState(D3D11->Common.SolidRasterStateNoCull);
    Context->RSSetViewports(1, &IrradianceViewport);
    {
        u32 Strides[] = {sizeof(vec3)};
        u32 Offsets[] = {0};
        Context->IASetVertexBuffers(0, 1, &D3D11->Common.CubemapVertexBuffer, Strides, Offsets);
    }
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(D3D11->Precompute.Layout);
    Context->VSSetShader(D3D11->Precompute.VertexShader, 0, 0);
    Context->PSSetShader(D3D11->Precompute.IrradiancePixelShader, 0, 0);
    Context->PSSetShaderResources(0, 1, &Cubemap.ResourceView);
    Context->PSSetSamplers(0, 1, &D3D11->Common.LinearSamplerState);
    
    vec4 IrradianceColors[6] = {
        vec4(1.0f, 0.0f, 0.0f, 1.0f), //0 RED    +x
        vec4(1.0f, 1.0f, 0.0f, 1.0f), //4 YELLOW -x
        vec4(0.0f, 1.0f, 0.0f, 1.0f), //1 GREEN  +y
        vec4(0.0f, 1.0f, 1.0f, 1.0f), //3 CYAN   -y
        vec4(0.0f, 0.0f, 1.0f, 1.0f), //2 BLUE   +z
        vec4(1.0f, 0.0f, 1.0f, 1.0f), //5 PINK   -z
    };
    
    for(u32 Index = 0; Index < 6; Index++)
    {
        ID3D11RenderTargetView* RenderTarget = IrradianceCube.RenderTargets[Index];
        Context->ClearRenderTargetView(RenderTarget, vec4(1.0f).e);
        Context->OMSetRenderTargets(1, &RenderTarget, 0);
        
        //Vertex constants
        VertexConstants.Projection = CaptureProj;
        VertexConstants.View = CaptureViews[Index];
        D3D11_FillConstantBuffers(Context, VertexConstantsBuffer, &VertexConstants, sizeof(VertexConstants));
        Context->VSSetConstantBuffers(0, 1, &VertexConstantsBuffer);
        
#if 0 //Debug irradiance colors
        vec4 Color = IrradianceColors[Index];
        D3D11_FillConstantBuffers(Context, PixelConstantsBuffer, &Color, sizeof(vec4));
        Context->PSSetConstantBuffers(0, 1, &PixelConstantsBuffer);
#endif
        
        Context->Draw(36, 0);
        
        //Free the render target view, we only need to draw to each texture once
        Context->OMSetRenderTargets(0, 0, 0);
        RenderTarget->Release();
    }
    
    //Specular cubemap
    s32 SpecularSize = SPECULAR_CUBEMAP_SIZE;
    d3d11_cubemap_target_mips SpecularCube = D3D11_CreateCubeTextureAndRenderTargetMips(Device, SpecularSize);
    
    Context->ClearState();
    
    Context->OMSetDepthStencilState(D3D11->Common.LessDepthStencilState, 0);
    Context->RSSetState(D3D11->Common.SolidRasterStateNoCull);
    {
        u32 Strides[] = {sizeof(vec3)};
        u32 Offsets[] = {0};
        Context->IASetVertexBuffers(0, 1, &D3D11->Common.CubemapVertexBuffer, Strides, Offsets);
    }
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(D3D11->Precompute.Layout);
    Context->VSSetShader(D3D11->Precompute.VertexShader, 0, 0);
    Context->PSSetShader(D3D11->Precompute.SpecularPixelShader, 0, 0);
    Context->GenerateMips(Cubemap.ResourceView);
    Context->PSSetShaderResources(0, 1, &Cubemap.ResourceView);
    Context->PSSetSamplers(0, 1, &D3D11->Common.LinearSamplerState);
    
    for(u32 MipIndex = 0; MipIndex < 5; MipIndex++)
    {
        for(u32 FaceIndex = 0; FaceIndex < 6; FaceIndex++)
        {
            D3D11_VIEWPORT SpecularViewport = {};
            // TODO: Fix this powf stuff into integer arithmetic for mips sizes
            SpecularViewport.Width = (f32)SpecularSize * powf(0.5, (f32)MipIndex);
            SpecularViewport.Height = (f32)SpecularSize * powf(0.5, (f32)MipIndex);
            SpecularViewport.MaxDepth = 1.0f;
            Context->RSSetViewports(1, &SpecularViewport);
            
            ID3D11RenderTargetView* RenderTarget = SpecularCube.RenderTargets[FaceIndex][MipIndex];
            Context->ClearRenderTargetView(RenderTarget, vec4(1.0f).e);
            Context->OMSetRenderTargets(1, &RenderTarget, 0);
            
            //Vertex constants
            VertexConstants.Projection = CaptureProj;
            VertexConstants.View = CaptureViews[FaceIndex];
            D3D11_FillConstantBuffers(Context, VertexConstantsBuffer, &VertexConstants, sizeof(VertexConstants));
            Context->VSSetConstantBuffers(0, 1, &VertexConstantsBuffer);
            
            float Roughness =  (float)MipIndex / 4;
            D3D11_FillConstantBuffers(Context, PixelConstantsBuffer, &Roughness, sizeof(float));
            Context->PSSetConstantBuffers(0, 1, &PixelConstantsBuffer);
            
            Context->Draw(36, 0);
            
            //Free the render target view, we only need to draw to each texture once
            Context->OMSetRenderTargets(0, 0, 0);
            RenderTarget->Release();
        }
    }
    
    d3d11_light_probe Result = {};
    
    Result.Base.ResourceView = Cubemap.ResourceView;
    Result.Base.Texture = Cubemap.Texture;
    
    Result.Irradiance.ResourceView = IrradianceCube.ResourceView;
    Result.Irradiance.Texture = IrradianceCube.Texture;
    
    Result.Specular.ResourceView = SpecularCube.ResourceView;
    Result.Specular.Texture = SpecularCube.Texture;
    
    return Result;
}

internal void*
D3D11_MapBuffer(ID3D11DeviceContext* Context, ID3D11Buffer* Buffer)
{
    D3D11_MAPPED_SUBRESOURCE Resource = {};
    HRESULT Result = Context->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
    Assert(Result == S_OK);
    
    return Resource.pData;
}

internal void
D3D11_UnmapBuffer(ID3D11DeviceContext* Context, ID3D11Buffer* Buffer)
{
    Context->Unmap(Buffer, 0);
}

internal void
D3D11_DebugDraw(d3d11_state* D3D11, mat4 CameraMatrix)
{
    ID3D11Device* Device = D3D11->Device;
    ID3D11DeviceContext* Context = D3D11->Context;
    
    //Fill the vertex buffers
    color_vertex* LinesWritePtr = (color_vertex*)D3D11_MapBuffer(Context, D3D11->Debug.LineVertices);
    for(u32 i = 0; i < Debug.LinesCount; i++)
    {
        debug_line* Line = Debug.Lines + i;
        
        vec4 Color = RGBAToVec4(Line->Color);
        *LinesWritePtr++ = { Line->A, Color };
        *LinesWritePtr++ = { Line->B, Color };
    }
    D3D11_UnmapBuffer(Context, D3D11->Debug.LineVertices);
    
    color_vertex* TrianglesWritePtr = (color_vertex*)D3D11_MapBuffer(Context, D3D11->Debug.TriangleVertices);
    for(u32 i = 0; i < Debug.TrianglesCount; i++)
    {
        debug_triangle* Triangle = Debug.Triangles + i;
        
        vec4 Color = RGBAToVec4(Triangle->Color);
        *TrianglesWritePtr++ = { Triangle->A, Color };
        *TrianglesWritePtr++ = { Triangle->B, Color };
        *TrianglesWritePtr++ = { Triangle->C, Color };
    }
    D3D11_UnmapBuffer(Context, D3D11->Debug.TriangleVertices);
    
    //Draw the lines
    Context->ClearState();
#if NEAR_FAR_PLANE_INVERSION
    Context->OMSetDepthStencilState(D3D11->Common.GreaterDepthStencilState, 0);
#else
    Context->OMSetDepthStencilState(D3D11->Common.LessDepthStencilState, 0);
#endif
    
    Context->OMSetRenderTargets(1, &D3D11->DefaultRenderTarget.RenderTarget, D3D11->DefaultDepthStencilView);
    Context->RSSetViewports(1, &D3D11->Viewport);
    Context->RSSetState(D3D11->Common.SolidRasterStateNoCull);
    
    u32 Stride = sizeof(color_vertex);
    u32 Offset = 0;
    Context->IASetVertexBuffers(0, 1, &D3D11->Debug.LineVertices, &Stride, &Offset);
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    Context->IASetInputLayout(D3D11->Debug.ColorLayout);
    
    Context->VSSetShader(D3D11->Debug.ColorVertex, 0, 0);
    Context->PSSetShader(D3D11->Debug.ColorPixel, 0, 0);
    
    mat4 Constants[2] = {
        mat4(1.0f),
        CameraMatrix,
    };
    D3D11_FillConstantBuffers(Context, D3D11->Debug.VertexConstantsBuffer, &Constants, sizeof(Constants));
    Context->VSSetConstantBuffers(0, 1, &D3D11->Debug.VertexConstantsBuffer);
    
    Context->Draw(Debug.LinesCount * 2, 0);
    
    //Bind the triangle vertex buffer and draw again
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetVertexBuffers(0, 1, &D3D11->Debug.TriangleVertices, &Stride, &Offset);
    Context->Draw(Debug.TrianglesCount * 3, 0);
}

internal void
D3D11_SetViewport(ID3D11DeviceContext* Context, s32 Width, s32 Height)
{
    D3D11_VIEWPORT Viewport = {};
    Viewport.Width = (float)Width;
    Viewport.Height = (float)Height;
    Viewport.MaxDepth = 1.0f;
    
    Context->RSSetViewports(1, &Viewport);
}

internal void
D3D11_DrawFrame(d3d11_state* D3D11)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    
    Context->RSSetState(D3D11->Common.SolidRasterStateNoCull);
    u32 Strides[] = {sizeof(vec3) + sizeof(vec2)};
    u32 Offsets[] = {0};
    Context->IASetVertexBuffers(0, 1, &D3D11->Common.FrameVertexBuffer, Strides, Offsets);
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(D3D11->Common.FrameLayout);
    Context->VSSetShader(D3D11->Common.FrameVertexShader, 0, 0);
    
    Context->Draw(6, 0);
}

internal void
D3D11_DrawLut(d3d11_state* D3D11, u32 Depth)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    
    Context->RSSetState(D3D11->Common.SolidRasterStateNoCull);
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Context->IASetInputLayout(0);
    Context->VSSetShader(D3D11->Atmosphere.LutVertexShader, 0, 0);
    Context->GSSetShader(D3D11->Atmosphere.LutGeometryShader, 0, 0);
    
    Context->DrawInstanced(3, Depth, 0, 0);
}

internal d3d11_atmosphere
D3D11_PrecomputeAtmosphere(d3d11_state* D3D11, atmosphere_parameters* Atmosphere)
{
    ID3D11Device* Device = D3D11->Device;
    ID3D11DeviceContext* Context = D3D11->Context;
    
    d3d11_atmosphere Result = {};
    Result.Transmittance = D3D11_CreateRenderTarget(Device, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, ATMOSPHERE_LUT_FORMAT);
    Result.Irradiance = D3D11_CreateRenderTarget(Device, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT,  ATMOSPHERE_LUT_FORMAT);
    Result.Scattering = D3D11_CreateRenderTarget3D(Device, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT,  SCATTERING_TEXTURE_DEPTH,  ATMOSPHERE_LUT_FORMAT);
    
    Result.ParametersBuffer = D3D11_CreateConstantBuffer(Device, sizeof(atmosphere_parameters));
    D3D11_FillConstantBuffers(Context, Result.ParametersBuffer, Atmosphere, sizeof(atmosphere_parameters));
    
    //Tansmittance
    Context->ClearState();
    Context->OMSetRenderTargets(1, &Result.Transmittance.RenderTarget, 0);
    Context->PSSetShader(D3D11->Atmosphere.TransmittanceShader, 0, 0);
    Context->PSSetConstantBuffers(0, 1, &Result.ParametersBuffer);
    D3D11_SetViewport(Context, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
    D3D11_DrawFrame(D3D11);
    
    //Direct irradiance
    Context->ClearState();
    Context->OMSetRenderTargets(1, &D3D11->Atmosphere.DeltaIrradiance.RenderTarget, 0);
    Context->PSSetShader(D3D11->Atmosphere.DirectIrradianceShader, 0, 0);
    Context->PSSetConstantBuffers(0, 1, &Result.ParametersBuffer);
    Context->PSSetSamplers(0, 1, &D3D11->Common.LinearSamplerState);
    Context->PSSetShaderResources(0, 1, &Result.Transmittance.ResourceView);
    D3D11_SetViewport(Context, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
    D3D11_DrawFrame(D3D11);
    
    //Single scattering
    Context->ClearState();
    ID3D11RenderTargetView* ScatteringTargets[3] = {
        D3D11->Atmosphere.DeltaScatteringR.RenderTarget,
        D3D11->Atmosphere.DeltaScatteringM.RenderTarget,
        Result.Scattering.RenderTarget,
    };
    Context->OMSetRenderTargets(3, ScatteringTargets, 0);
    Context->PSSetShader(D3D11->Atmosphere.SingleScatterShader, 0, 0);
    Context->PSSetConstantBuffers(0, 1, &Result.ParametersBuffer);
    Context->PSSetSamplers(0, 1, &D3D11->Common.LinearSamplerState);
    Context->PSSetShaderResources(0, 1, &Result.Transmittance.ResourceView);
    D3D11_SetViewport(Context, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
    D3D11_DrawLut(D3D11, SCATTERING_TEXTURE_DEPTH);
    
    //Multiple scattering
    for(u32 ScatteringOrder = 2; ScatteringOrder <= MAX_SCATTERING_ORDER; ScatteringOrder++)
    {
        //Scattering Density
        Context->ClearState();
        s32 ScatOrder = ScatteringOrder;
        D3D11_FillConstantBuffers(Context, D3D11->Atmosphere.ScatteringOrderBuffer, &ScatOrder, sizeof(s32));
        
        Context->OMSetRenderTargets(1, &D3D11->Atmosphere.DeltaScatteringDensity.RenderTarget, 0);
        Context->PSSetShader(D3D11->Atmosphere.ScatteringDensityShader, 0, 0);
        Context->PSSetConstantBuffers(0, 1, &Result.ParametersBuffer);
        Context->PSSetConstantBuffers(1, 1, &D3D11->Atmosphere.ScatteringOrderBuffer);
        Context->PSSetSamplers(0, 1, &D3D11->Common.LinearSamplerState);
        {
            ID3D11ShaderResourceView* ShaderResources [] = {
                Result.Transmittance.ResourceView,
                D3D11->Atmosphere.DeltaIrradiance.ResourceView,
                D3D11->Atmosphere.DeltaScatteringR.ResourceView,
                D3D11->Atmosphere.DeltaScatteringM.ResourceView,
                D3D11->Atmosphere.DeltaScatteringR.ResourceView,
            };
            Context->PSSetShaderResources(0, ArrayCount(ShaderResources), ShaderResources);
        }
        D3D11_SetViewport(Context, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
        D3D11_DrawLut(D3D11, SCATTERING_TEXTURE_DEPTH);
        
        //Indirect irradiance
        Context->ClearState();
        ScatOrder = ScatteringOrder - 1;
        D3D11_FillConstantBuffers(Context, D3D11->Atmosphere.ScatteringOrderBuffer, &ScatOrder, sizeof(s32));
        
        ID3D11RenderTargetView* IrradianceRenderTargets[2] = {
            D3D11->Atmosphere.DeltaIrradiance.RenderTarget,
            Result.Irradiance.RenderTarget
        };
        Context->OMSetRenderTargets(2, IrradianceRenderTargets, 0);
        Context->OMSetBlendState(D3D11->Atmosphere.Blend0Nop1Add, 0, 0xFFFFFFFF);
        Context->PSSetShader(D3D11->Atmosphere.IndirectIrradianceShader, 0, 0);
        Context->PSSetConstantBuffers(0, 1, &Result.ParametersBuffer);
        Context->PSSetConstantBuffers(1, 1, &D3D11->Atmosphere.ScatteringOrderBuffer);
        Context->PSSetSamplers(0, 1, &D3D11->Common.LinearSamplerState);
        {
            ID3D11ShaderResourceView* ShaderResources [] = {
                D3D11->Atmosphere.DeltaScatteringR.ResourceView,
                D3D11->Atmosphere.DeltaScatteringM.ResourceView,
                D3D11->Atmosphere.DeltaScatteringR.ResourceView, //TODO: Take this out
            };
            Context->PSSetShaderResources(0, ArrayCount(ShaderResources), ShaderResources);
        }
        D3D11_SetViewport(Context, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
        D3D11_DrawFrame(D3D11);
        
        //Multiple scattering
        Context->ClearState();
        ScatOrder = ScatteringOrder;
        D3D11_FillConstantBuffers(Context, D3D11->Atmosphere.ScatteringOrderBuffer, &ScatOrder, sizeof(s32));
        
        ID3D11RenderTargetView* MultiScatteringRenderTargets[2] = {
            D3D11->Atmosphere.DeltaScatteringR.RenderTarget,
            Result.Scattering.RenderTarget,
        };
        Context->OMSetRenderTargets(2, MultiScatteringRenderTargets, 0);
        Context->OMSetBlendState(D3D11->Atmosphere.Blend0Nop1Add, 0, 0xFFFFFFFF);
        Context->PSSetShader(D3D11->Atmosphere.MultipleScatteringShader, 0, 0);
        Context->PSSetConstantBuffers(0, 1, &Result.ParametersBuffer);
        Context->PSSetConstantBuffers(1, 1, &D3D11->Atmosphere.ScatteringOrderBuffer);
        Context->PSSetSamplers(0, 1, &D3D11->Common.LinearSamplerState);
        {
            ID3D11ShaderResourceView* ShaderResources [] = {
                Result.Transmittance.ResourceView,
                D3D11->Atmosphere.DeltaScatteringDensity.ResourceView,
            };
            Context->PSSetShaderResources(0, ArrayCount(ShaderResources), ShaderResources);
        }
        D3D11_SetViewport(Context, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
        D3D11_DrawLut(D3D11, SCATTERING_TEXTURE_DEPTH);
    }
    
    return Result;
}

internal image_data
D3D11_ReadTexture2D(d3d11_state* D3D11, ID3D11Texture2D* Texture)
{
    ID3D11Device* Device = D3D11->Device;
    ID3D11DeviceContext* Context = D3D11->Context;
    
    D3D11_TEXTURE2D_DESC Desc = {};
    Texture->GetDesc(&Desc);
    Assert(Desc.ArraySize == 1 && Desc.SampleDesc.Count == 1);
    Desc.Usage = D3D11_USAGE_STAGING;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    Desc.BindFlags = 0;
    
    ID3D11Texture2D* Mappable = 0;
    HRESULT HResult = Device->CreateTexture2D(&Desc, 0, &Mappable);
    Assert(HResult == S_OK);
    
    Context->CopyResource(Mappable, Texture);
    
    D3D11_MAPPED_SUBRESOURCE MappedResource = {};
    HResult = Context->Map(Mappable, 0, D3D11_MAP_READ, 0, &MappedResource);
    Assert(HResult == S_OK);
    
    u32 BytesPerPixel = MappedResource.RowPitch / Desc.Width;
    //We don't support padding, luckily we usually don't need it
    Assert(MappedResource.RowPitch == Desc.Width * BytesPerPixel);
    
    u32 ImageSize = MappedResource.RowPitch * Desc.Height;
    u8* ImageData = (u8*)ZeroAlloc(ImageSize);
    memcpy(ImageData, MappedResource.pData, ImageSize);
    Context->Unmap(Mappable, 0);
    
    image_data Result = CreateImage(ImageData, Desc.Width, Desc.Height, MappedResource.RowPitch, BytesPerPixel);
    return Result;
}

internal image_3d_data
D3D11_ReadTexture3D(d3d11_state* D3D11, ID3D11Texture3D* Texture)
{
    ID3D11Device* Device = D3D11->Device;
    ID3D11DeviceContext* Context = D3D11->Context;
    
    D3D11_TEXTURE3D_DESC Desc = {};
    Texture->GetDesc(&Desc);
    Desc.MipLevels = 1;
    Desc.Usage = D3D11_USAGE_STAGING;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    Desc.BindFlags = 0;
    
    ID3D11Texture3D* Mappable = 0;
    HRESULT HResult = Device->CreateTexture3D(&Desc, 0, &Mappable);
    Assert(HResult == S_OK);
    
    Context->CopyResource(Mappable, Texture);
    
    D3D11_MAPPED_SUBRESOURCE MappedResource = {};
    HResult = Context->Map(Mappable, 0, D3D11_MAP_READ, 0, &MappedResource);
    Assert(HResult == S_OK);
    
    u32 BytesPerPixel = MappedResource.RowPitch / Desc.Width;
    //We don't support padding, luckily we usually don't need it
    Assert(MappedResource.RowPitch == Desc.Width * BytesPerPixel);
    Assert(MappedResource.DepthPitch == MappedResource.RowPitch * Desc.Height);
    
    u32 ImageSize = MappedResource.RowPitch * Desc.Height * Desc.Depth;
    u8* ImageData = (u8*)ZeroAlloc(ImageSize);
    memcpy(ImageData, MappedResource.pData, ImageSize);
    Context->Unmap(Mappable, 0);
    
    image_3d_data Result = CreateImage3D(ImageData, Desc.Width, Desc.Height, Desc.Depth, BytesPerPixel);
    return Result;
}

internal void
D3D11_PrecompouteAndExportAtmosphere(d3d11_state* D3D11, atmosphere_parameters* Parameters, char* Path)
{
    d3d11_atmosphere Atmosphere = D3D11_PrecomputeAtmosphere(D3D11, Parameters);
    image_data TransmittanceImage = D3D11_ReadTexture2D(D3D11, Atmosphere.Transmittance.Texture);
    image_data IrradianceImage    = D3D11_ReadTexture2D(D3D11, Atmosphere.Irradiance.Texture);
    image_3d_data ScatteringImage = D3D11_ReadTexture3D(D3D11, Atmosphere.Scattering.Texture);
    
    HANDLE FileHandle = CreateFile(Path, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
    
    b32 Success = false;
    DWORD BytesWriten = 0;
    
    u32 ParamsSize = ALIGN_UP(sizeof(atmosphere_parameters), 16);
    Success = WriteFile(FileHandle, Parameters, ParamsSize, &BytesWriten, 0);
    Assert(Success && BytesWriten == ParamsSize);
    
    u32 TransmittanceSize = TransmittanceImage.Pitch * TransmittanceImage.Height;
    Assert(TransmittanceSize == TRANSMITTANCE_TEXTURE_WIDTH * TRANSMITTANCE_TEXTURE_HEIGHT * ATMOSPHERE_LUT_FORMAT_SIZE);
    Success = WriteFile(FileHandle, TransmittanceImage.Data, TransmittanceSize, &BytesWriten, 0);
    Assert(Success && BytesWriten == TransmittanceSize);
    
    u32 IrradianceSize = IrradianceImage.Pitch * IrradianceImage.Height;
    Assert(IrradianceSize == IRRADIANCE_TEXTURE_WIDTH * IRRADIANCE_TEXTURE_HEIGHT * ATMOSPHERE_LUT_FORMAT_SIZE);
    Success = WriteFile(FileHandle, IrradianceImage.Data, IrradianceSize, &BytesWriten, 0);
    Assert(Success && BytesWriten == IrradianceSize);
    
    u32 ScatteringSize = ScatteringImage.Width * ScatteringImage.Height * 
        ScatteringImage.Depth * ScatteringImage.BytesPerPixel;
    Assert(ScatteringSize == SCATTERING_TEXTURE_WIDTH * SCATTERING_TEXTURE_HEIGHT * SCATTERING_TEXTURE_WIDTH * ATMOSPHERE_LUT_FORMAT_SIZE);
    Success = WriteFile(FileHandle, ScatteringImage.Data, ScatteringSize, &BytesWriten, 0);
    Assert(Success && BytesWriten == ScatteringSize);
    
    Free(TransmittanceImage.Data);
    Free(IrradianceImage.Data);
    Free(ScatteringImage.Data);
    
    CloseHandle(FileHandle);
}

internal void* Win32_ReadEntireFile(char*, u32*);
internal void Win32_FreeFileMemory(void*);

internal d3d11_atmosphere
D3D11_LoadAtmosphereFromFile(d3d11_state* D3D11, char* Path)
{
    ID3D11Device* Device = D3D11->Device;
    ID3D11DeviceContext* Context = D3D11->Context;
    
    u32 Size = 0;
    u8* Data = (u8*)Win32_ReadEntireFile(Path, &Size);
    Assert(Data && Size);
    u8* End = Data + Size;
    
    u8* It = Data;
    u32 ParamsSize = ALIGN_UP(sizeof(atmosphere_parameters), 16);
    Assert(It + ParamsSize <= End);
    
    d3d11_atmosphere Result = {};
    Result.ParametersBuffer = D3D11_CreateConstantBuffer(Device, sizeof(atmosphere_parameters));
    D3D11_FillConstantBuffers(Context, Result.ParametersBuffer, It, sizeof(atmosphere_parameters));
    
    It += ParamsSize;
    
    u32 TransmittanceSize = TRANSMITTANCE_TEXTURE_WIDTH * TRANSMITTANCE_TEXTURE_HEIGHT * ATMOSPHERE_LUT_FORMAT_SIZE;
    Assert(It + TransmittanceSize <= End);
    
    image_data TransmittanceImage = CreateImage(It, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, 
                                                ATMOSPHERE_LUT_FORMAT_SIZE * TRANSMITTANCE_TEXTURE_WIDTH, ATMOSPHERE_LUT_FORMAT_SIZE);
    d3d11_texture Transmittance = D3D11_CreateTexture(Device, &TransmittanceImage, ATMOSPHERE_LUT_FORMAT);
    Result.Transmittance.Texture = Transmittance.Texture;
    Result.Transmittance.ResourceView = Transmittance.ResourceView;
    Result.Transmittance.Width = TRANSMITTANCE_TEXTURE_WIDTH;
    Result.Transmittance.Height = TRANSMITTANCE_TEXTURE_HEIGHT;

    It += TransmittanceSize;
    
    
    u32 IrradianceSize = IRRADIANCE_TEXTURE_WIDTH * IRRADIANCE_TEXTURE_HEIGHT * ATMOSPHERE_LUT_FORMAT_SIZE;
    Assert(It + IrradianceSize <= End);
    
    image_data IrradianceImage = CreateImage(It, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, 
                                             ATMOSPHERE_LUT_FORMAT_SIZE * IRRADIANCE_TEXTURE_WIDTH, ATMOSPHERE_LUT_FORMAT_SIZE);
    d3d11_texture Irradiance = D3D11_CreateTexture(Device, &IrradianceImage, ATMOSPHERE_LUT_FORMAT);
    Result.Irradiance.Texture = Irradiance.Texture;
    Result.Irradiance.ResourceView = Irradiance.ResourceView;
    Result.Irradiance.Width = IRRADIANCE_TEXTURE_WIDTH;
    Result.Irradiance.Height = IRRADIANCE_TEXTURE_HEIGHT;
    
    It += IrradianceSize;
    
    u32 ScatteringSize = SCATTERING_TEXTURE_WIDTH * SCATTERING_TEXTURE_HEIGHT * SCATTERING_TEXTURE_DEPTH * ATMOSPHERE_LUT_FORMAT_SIZE;
    Assert(It + ScatteringSize <= End);
    
    image_3d_data ScatteringImage = CreateImage3D(It, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, 
                                               SCATTERING_TEXTURE_DEPTH, ATMOSPHERE_LUT_FORMAT_SIZE);
    d3d11_texture_3d Scattering = D3D11_CreateTexture3D(Device, &ScatteringImage, ATMOSPHERE_LUT_FORMAT);
    Result.Scattering.Texture = Scattering.Texture;
    Result.Scattering.ResourceView = Scattering.ResourceView;
    Result.Scattering.Width = SCATTERING_TEXTURE_WIDTH;
    Result.Scattering.Height = SCATTERING_TEXTURE_HEIGHT;
    Result.Scattering.Depth = SCATTERING_TEXTURE_DEPTH;
    
    It += ScatteringSize;
    Assert(It == End);
    
    return Result;
}
