internal void
BindAndDrawMeshForShadows(d3d11_state* D3D11, mesh_gpu* GpuMesh)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    
    u32 Strides[] = { sizeof(vec3) };
    u32 Offsets[] = { 0 };
    Context->IASetVertexBuffers(0, 1, &GpuMesh->PositionsBuffer, Strides, Offsets);
    Context->IASetPrimitiveTopology(GpuMesh->Flags & MESH_IS_STRIP ? 
                                    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: 
                                    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    //If we have indices we bind em
    if(!(GpuMesh->Flags & MESH_NO_INDICES))
    {
        Context->IASetIndexBuffer(GpuMesh->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }
    
    //Draw
    if(GpuMesh->Flags & MESH_NO_INDICES)
    {
        Context->Draw(GpuMesh->VerticesCount, 0);
    }
    else
    {
        Context->DrawIndexed(GpuMesh->IndicesCount, 0, 0);
    }
}

internal void
DrawMeshesToShadowMaps(d3d11_state* D3D11, scene* Scene)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    
    //Commmon to all meshes and lights
    Context->ClearState();
    Context->OMSetDepthStencilState(D3D11->Common.LessDepthStencilState, 0);
    Context->RSSetState(D3D11->Shadow.RasterState);
    Context->IASetInputLayout(D3D11->Shadow.Layout);
    Context->VSSetShader(D3D11->Shadow.VertexShader, 0, 0);
    Context->VSSetConstantBuffers(0, 1, &D3D11->Shadow.VertexConstantsBuffer);
    Context->PSSetShader(0, 0, 0);
    
    d3d11_shadow_vertex_constants VertexConstants;
    
    for(u32 LightIndex = 0; LightIndex < Scene->DirectionalLightsCount; LightIndex++)
    {
        //Bind light
        directional_light* Light = Scene->DirectionalLights + LightIndex;
        d3d11_shadow_map ShadowMap = Light->ShadowMap;
        
        Context->ClearDepthStencilView(ShadowMap.DepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
        D3D11_VIEWPORT Viewport = {};
        Viewport.Width = (f32)ShadowMap.Width;
        Viewport.Height = (f32)ShadowMap.Height;
        Viewport.MaxDepth = 1.0f;
        Context->RSSetViewports(1, &Viewport);
        Context->OMSetRenderTargets(0, 0, ShadowMap.DepthView);
        
        VertexConstants.Shadow = GetShadowMatrix(Light);
        
        if(Light->DebugDrawFrustum) 
        {
            DebugFrustum(VertexConstants.Shadow, RGB(255, 0, 255));
        }
        
        for(u32 MeshIndex = 0; MeshIndex < Scene->MeshesCount; MeshIndex++)
        {
            //Bind mesh
            mesh* Mesh = Scene->Meshes + MeshIndex;
            if(!Mesh->CastsShadows)
                continue;
            
            VertexConstants.Model = Mesh->DrawTransform;
            D3D11_FillConstantBuffers(Context, D3D11->Shadow.VertexConstantsBuffer, &VertexConstants, sizeof(VertexConstants));
            
            BindAndDrawMeshForShadows(D3D11, Mesh->GpuMesh);
        }
    }
}

internal void
DrawMeshesToShadowCubemaps(d3d11_state* D3D11, scene* Scene)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    
    //Commmon to all meshes and lights
    Context->ClearState();
    Context->OMSetDepthStencilState(D3D11->Common.LessDepthStencilState, 0);
    
    //NOTE: We use the default rasterizer because we use a Left Handed camera matrix to capture
    //the shadowmap, this results in our meshes showing reverse winding. Since we want
    //to capture the backfaces anyways we can use our default rasterizer
    Context->RSSetState(D3D11->Common.SolidRasterState);
    Context->IASetInputLayout(D3D11->Shadow.Layout);
    Context->VSSetShader(D3D11->Shadow.CubemapVertexShader, 0, 0);
    Context->VSSetConstantBuffers(0, 1, &D3D11->Shadow.VertexConstantsBuffer);
    Context->PSSetShader(D3D11->Shadow.CubemapPixelShader, 0, 0);
    Context->PSSetConstantBuffers(0, 1, &D3D11->Shadow.PixelConstantsBuffer);
    
    d3d11_shadow_vertex_constants VertexConstants;
    d3d11_shadow_pixel_constants PixelConstants;
    
    //Capture matrices
    vec3 Axis[] = {
        vec3( 1.0f,  0.0f,  0.0f),//0 +x
        vec3(-1.0f,  0.0f,  0.0f),//1 -x
        vec3( 0.0f,  1.0f,  0.0f),//2 +y
        vec3( 0.0f, -1.0f,  0.0f),//3 -y
        vec3( 0.0f,  0.0f,  1.0f),//4 +z
        vec3( 0.0f,  0.0f, -1.0f),//5 -z
    };
    
    vec3 Up[] = {
        vec3(0.0f,  1.0f,  0.0f), //0 +x
        vec3(0.0f,  1.0f,  0.0f), //1 -x
        vec3(0.0f,  0.0f, -1.0f), //2 +y
        vec3(0.0f,  0.0f,  1.0f), //3 -y
        vec3(0.0f,  1.0f,  0.0f), //4 +z
        vec3(0.0f,  1.0f,  0.0f), //5 -z
    };
    
    
    s32 Counter = 0;
    for(u32 LightIndex = 0; LightIndex < Scene->PointLightsCount; LightIndex++)
    {
        point_light* Light = Scene->PointLights + LightIndex;
        d3d11_shadow_cubemap Cubemap = Light->ShadowCubemap;
        
        PixelConstants.LightPosition = Light->Position;
        PixelConstants.FarPlane = Light->Radius;
        D3D11_FillConstantBuffers(Context, D3D11->Shadow.PixelConstantsBuffer, &PixelConstants, sizeof(PixelConstants));
        
        if(Light->DebugDrawLightPosition)
        {
            DebugPoint(Light->Position, RGB(0,0,0));
        }
        
        for(s32 FaceIndex = 0; FaceIndex < 6; FaceIndex++)
        {
            mat4 CaptureProj = Mat4PerspectiveLH(90.0f, 0.1f, Light->Radius, 1.0f);
            mat4 CaptureView = Mat4LookAtLH(Light->Position, Light->Position + Axis[FaceIndex], Up[FaceIndex]);
            mat4 ShadowMatrix = CaptureProj * CaptureView;
            VertexConstants.Shadow = ShadowMatrix;
            
            //Skip this frustum if it doesn't intersect the camera frustum
            frustum Frustum = FrustumFromMatrix(ShadowMatrix);
            if(InspectorData.FrustumFrustumCulling && !FrustumFrustumIntersection(&Frustum, &Scene->CameraFrustum))
            {
                continue;
            }
            
            if(Light->DebugDrawFrustum && Light->DebugFrustumFace == FaceIndex) 
            {
                mat4 DebugView = Mat4LookAt(Light->Position, Light->Position + Axis[FaceIndex], Up[FaceIndex]);
                mat4 DebugProj = Mat4Perspective(90.0f, 0.1f, Light->Radius, 1.0f);
                DebugFrustum(DebugProj * DebugView, RGB(255,0,255));
            }
            
            ID3D11DepthStencilView* DepthView = Cubemap.DepthViews[FaceIndex];
            Context->ClearDepthStencilView(DepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
            
            D3D11_VIEWPORT Viewport = {};
            Viewport.Width = (f32)Cubemap.Size;
            Viewport.Height = (f32)Cubemap.Size;
            Viewport.MaxDepth = 1.0f;
            Context->RSSetViewports(1, &Viewport);
            Context->OMSetRenderTargets(0, 0, DepthView);
            
            for(u32 MeshIndex = 0; MeshIndex < Scene->MeshesCount; MeshIndex++)
            {
                //Bind mesh
                mesh* Mesh = Scene->Meshes + MeshIndex;
                if(!Mesh->CastsShadows)
                    continue;
                
                if(!IsAABBInsideFrustum(Mesh->AABB, Frustum.Planes))
                {
                    if(Light->DebugDrawFrustum && Light->DebugFrustumFace == FaceIndex)
                        DebugAABB(Mesh->AABB.Min, Mesh->AABB.Max, RGB(255, 0, 0));
                    
                    if(InspectorData.ShadowCubemapFrustum)
                    {
                        continue;
                    }
                }
                else
                {
                    if(Light->DebugDrawFrustum && Light->DebugFrustumFace == FaceIndex)
                        DebugAABB(Mesh->AABB.Min, Mesh->AABB.Max, RGB(0, 255, 0));
                }
                
                Counter++;
                VertexConstants.Model = Mesh->DrawTransform;
                D3D11_FillConstantBuffers(Context, D3D11->Shadow.VertexConstantsBuffer, &VertexConstants, sizeof(VertexConstants));
                
                BindAndDrawMeshForShadows(D3D11, Mesh->GpuMesh);
            }
        }
    }
    
    InspectorData.ObjectsDrawnOnCubemap = Counter;
}

internal void
DrawMeshes(d3d11_state* D3D11, scene* Scene, bool DepthOnly)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    
    Context->ClearState();
    
    if(DepthOnly || !InspectorData.DepthPrepass)
    {
#if NEAR_FAR_PLANE_INVERSION
        Context->OMSetDepthStencilState(D3D11->Common.GreaterDepthStencilState, 0);
#else
        Context->OMSetDepthStencilState(D3D11->Common.LessDepthStencilState, 0);
#endif
    }
    else
    {
        Context->OMSetDepthStencilState(D3D11->Common.EqualDepthStencilState, 0);
    }
    
    Context->OMSetRenderTargets(1, &D3D11->PBR.IntermediateTarget.RenderTarget, D3D11->DefaultDepthStencilView);
    
    Context->RSSetViewports(1, &D3D11->Viewport);
    Context->RSSetState(D3D11->Common.SolidRasterState);
    
    Context->VSSetShader(D3D11->PBR.VertexShader, 0, 0);
    Context->VSSetConstantBuffers(0, 1, &D3D11->PBR.VertexConstantsBuffer);
    
    if(!DepthOnly)
    {
        Context->PSSetShader(D3D11->PBR.PixelShader, 0, 0);
        Context->PSSetConstantBuffers(0, 1, &D3D11->PBR.PixelConstantsBuffer);
        
        ID3D11SamplerState* Samplers[] = {
            D3D11->Common.LinearSamplerState,
            D3D11->Common.ShadowSamplerState,
        };
        Context->PSSetSamplers(0, ArrayCount(Samplers), Samplers);
    }
    
    Context->IASetInputLayout(D3D11->PBR.Layout);
    
    d3d11_pbr_vertex_constants VertexConstants = {};
    VertexConstants.Projection = Scene->Projection;
    VertexConstants.View = Scene->View;
    for(u32 i = 0; i < Scene->DirectionalLightsCount; i++)
    {
        VertexConstants.ShadowMatrix[i] = GetShadowMatrix(Scene->DirectionalLights + i);
    }
    
    d3d11_pbr_pixel_constants PixelConstants = {};
    
    if(!DepthOnly)
    {
        for(u32 i = 0; i < Scene->PointLightsCount; i++)
        {
            point_light* Light = Scene->PointLights + i;
            PixelConstants.PointLight[i].Position = Light->Position;
            PixelConstants.PointLight[i].Radius = Light->Radius;
            PixelConstants.PointLight[i].Color = Light->Color;
        }
        
        for(u32 i = 0; i < Scene->DirectionalLightsCount; i++)
        {
            directional_light* Light = Scene->DirectionalLights + i;
            PixelConstants.DirectionalLight[i].Direction = Light->Direction;
            PixelConstants.DirectionalLight[i].Color = Light->Color;
        }
        PixelConstants.ViewPos = Scene->ViewPosition;
        PixelConstants.ExposureEnabled = false;
        PixelConstants.Exposure = 1.0f;
        PixelConstants.MinBias = MIN_SHADOW_BIAS;
        PixelConstants.MaxBias = MAX_SHADOW_BIAS;
    }
    
    u32 Counter = 0;
    for(u32 MeshIndex = 0; MeshIndex < Scene->MeshesCount; MeshIndex++)
    {
        //Bind mesh
        mesh* Mesh = Scene->Meshes + MeshIndex;
        mesh_gpu* GpuMesh = Mesh->GpuMesh;
        
        if(InspectorData.FrustumCulling && !IsAABBInsideFrustum(Mesh->AABB, Scene->CameraFrustum.Planes))
            continue;   
        
        u32 Strides[] = {sizeof(vec3), sizeof(vec3), sizeof(vec3), sizeof(vec2)};
        u32 Offsets[] = {0, 0, 0, 0};
        Context->IASetVertexBuffers(0, 4, GpuMesh->VertexBuffers, Strides, Offsets);
        Context->IASetPrimitiveTopology(GpuMesh->Flags & MESH_IS_STRIP ? 
                                        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP: 
                                        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        //If we have indices we bind em
        if(!(GpuMesh->Flags & MESH_NO_INDICES))
        {
            Context->IASetIndexBuffer(GpuMesh->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        }
        
        VertexConstants.Model = Mesh->DrawTransform;
        VertexConstants.NormalMatrix = Mat4NormalMatrix(Mesh->DrawTransform);
        D3D11_FillConstantBuffers(Context, D3D11->PBR.VertexConstantsBuffer, &VertexConstants, sizeof(VertexConstants));
        
        if(!DepthOnly)
        {
            //Set material
            Assert(Mesh->MaterialIndex < Scene->MaterialsCount);
            material* Material = Scene->Materials + Mesh->MaterialIndex;
            PixelConstants.HasAlbedo = Material->HasAlbedo;
            PixelConstants.HasNormal = Material->HasNormal;
            PixelConstants.HasRoughness = Material->HasRoughness;
            PixelConstants.HasMetallic = Material->HasMetallic;
            PixelConstants.HasAO = Material->HasAO;
            PixelConstants.HasEmissive = Material->HasEmissive;
            PixelConstants.AlbedoConst = Material->Albedo;
            PixelConstants.MetallicConst = Material->Metallic;
            PixelConstants.RoughnessConst = Material->Roughness;
            PixelConstants.AOConst = 1.0f;
            D3D11_FillConstantBuffers(Context, D3D11->PBR.PixelConstantsBuffer, &PixelConstants, sizeof(PixelConstants));
            
            //Textures
            ID3D11ShaderResourceView* Textures[9 + MAX_DIRECTIONAL_LIGHTS_COUNT + MAX_POINT_LIGHTS_COUNT] = {
                Material->AlbedoTexture.ResourceView,
                Material->MetallicTexture.ResourceView,
                Material->RoughnessTexture.ResourceView,
                Material->NormalTexture.ResourceView,
                Material->AOTexture.ResourceView,
                Material->EmissiveTexture.ResourceView,
                Scene->Probe.Irradiance.ResourceView,
                Scene->Probe.Specular.ResourceView,
                D3D11->PBR.BRDFTexture.ResourceView,
            };
            for(u32 i = 0; i < MAX_DIRECTIONAL_LIGHTS_COUNT; i++)
            {
                Textures[9 + i] = Scene->DirectionalLights[i].ShadowMap.ResourceView;
            }
            for(u32 i = 0; i < MAX_DIRECTIONAL_LIGHTS_COUNT; i++)
            {
                Textures[9 + MAX_DIRECTIONAL_LIGHTS_COUNT + i] = Scene->PointLights[i].ShadowCubemap.ResourceView;
            }
            
            Context->PSSetShaderResources(0, ArrayCount(Textures), Textures);
        }
        
        //Draw
        if(GpuMesh->Flags & MESH_NO_INDICES)
        {
            Context->Draw(GpuMesh->VerticesCount, 0);
        }
        else
        {
            Context->DrawIndexed(GpuMesh->IndicesCount, 0, 0);
        }
        
        Counter++;
    }
    
    InspectorData.ObjectsDrawn = Counter;
}

internal void
DrawAtmosphere(d3d11_state* D3D11, scene* Scene)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    d3d11_atmosphere Atmosphere = Scene->Atmosphere;
    
    d3d11_atmosphere_render_constants Constants = {};
    Constants.CameraPos = Scene->ViewPosition;
    Constants.SunDirection = Scene->SunDirection;
    Constants.SunIlluminance = Scene->SunIlluminanceScale * Scene->SunIlluminanceColor;
    Constants.AerialPerspectiveScale = InspectorData.AerialPerspectiveScale;
    
    Constants.CameraPos = Scene->ViewPosition;
    Constants.CameraForward = Scene->CameraForward;
    Constants.PerspectiveHor = Scene->PerspectiveHor;
    Constants.PerspectiveVer = Scene->PerspectiveVer;
    Constants.PerspectiveA = Scene->CameraFar / (Scene->CameraFar - Scene->CameraNear);
    Constants.PerspectiveB = (-Scene->CameraFar * Scene->CameraNear) / (Scene->CameraFar - Scene->CameraNear);
    
    D3D11_FillConstantBuffers(Context, D3D11->Atmosphere.RenderAtmosphereBuffer, &Constants, sizeof(Constants));
    
    Context->ClearState();
    Context->OMSetRenderTargets(1, &D3D11->PBR.IntermediateTarget.RenderTarget, 0);
    Context->OMSetBlendState(D3D11->Atmosphere.RenderBlend, 0, 0xFFFFFFFF);
    Context->PSSetShader(D3D11->Atmosphere.RenderAtmosphereShader, 0, 0);
    Context->PSSetConstantBuffers(0, 1, &Atmosphere.ParametersBuffer);
    Context->PSSetConstantBuffers(2, 1, &D3D11->Atmosphere.RenderAtmosphereBuffer);
    Context->PSSetShaderResources(0, 1, &Atmosphere.Transmittance.ResourceView);
    Context->PSSetShaderResources(1, 1, &Atmosphere.Irradiance.ResourceView);
    Context->PSSetShaderResources(2, 1, &Atmosphere.Scattering.ResourceView);
    Context->PSSetShaderResources(3, 1, &D3D11->DefaultDepthStencilResourceView);
    Context->PSSetSamplers(0, 1, &D3D11->Common.LinearSamplerState);
    Context->RSSetViewports(1, &D3D11->Viewport);
    D3D11_DrawFrame(D3D11);
}


internal void
DrawPostprocess(d3d11_state* D3D11, scene* Scene)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    
    Context->ClearState();
    Context->OMSetRenderTargets(1, &D3D11->DefaultRenderTarget.RenderTarget, 0);
    Context->PSSetShader(D3D11->PBR.PostprocessShader, 0, 0);
    Context->PSSetShaderResources(0, 1, &D3D11->PBR.IntermediateTarget.ResourceView);
    Context->RSSetViewports(1, &D3D11->Viewport);
    D3D11_DrawFrame(D3D11);
}

internal void
D3D11_DrawScene(d3d11_state* D3D11, scene* Scene)
{
    //Compute mesh draw transforms from position
    for(u32 i = 0; i < Scene->MeshesCount; i++)
    {
        mesh* Mesh = Scene->Meshes + i;
        
        mat3 Scale = Mat3Scale(Mesh->Scale);
        mat3 Rotation = Mat3FromEulerXYZ(Mesh->Rotation);
        mat3 Transform = Rotation * Scale;
        Mesh->DrawTransform = Mat4FromMat3AndTranslation(Transform, Mesh->Position);
        
        mesh_data* MeshData = Mesh->MeshData;
        if(Mesh->AABBDirty)
        {
            Mesh->AABB = ComputeAABB(MeshData->Positions, MeshData->VerticesCount, Transform, Mesh->Position);
            Mesh->AABBDirty = false;
        }
        //    DebugAABB(Mesh->AABB.Min, Mesh->AABB.Max, RGB(255,0,0));
    }
    
    Scene->CameraFrustum = FrustumFromMatrix(Scene->Projection * Scene->View);
    //Clear intermediate target
    D3D11->Context->ClearRenderTargetView(D3D11->PBR.IntermediateTarget.RenderTarget, vec4(0.0f, 0.0f, 0.0, 0.0f).e);
    
    {
        D3D11_PROFILE_BLOCK(D3D11, D3D11_PROFILE_SHADOWMAP);
        DrawMeshesToShadowMaps(D3D11, Scene);
    }
    
    {
        D3D11_PROFILE_BLOCK(D3D11, D3D11_PROFILE_SHADOWCUBEMAP);
        DrawMeshesToShadowCubemaps(D3D11, Scene);
    }
    
    {
        D3D11_PROFILE_BLOCK(D3D11, D3D11_PROFILE_DEPTH_PREPASS);
        if(InspectorData.DepthPrepass)
            DrawMeshes(D3D11, Scene, true);
    }
    
    {
        D3D11_PROFILE_BLOCK(D3D11, D3D11_PROFILE_MESHES);
        DrawMeshes(D3D11, Scene, false);
    }
    
    {
        D3D11_PROFILE_BLOCK(D3D11, D3D11_PROFILE_ATMOSPHERE);
        DrawAtmosphere(D3D11, Scene);
    }
    
    {
        D3D11_PROFILE_BLOCK(D3D11, D3D11_PROFILE_POSTPROCESS);
        DrawPostprocess(D3D11, Scene);
    }    
}