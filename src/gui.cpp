#include "gui.h"

internal void
DrawStats(vec2 Position, f32 CpuSecondsElapsed, f32 GpuSecondsElapsed)
{
    static f32 CpuFrameTimes[32];
    static f32 GpuFrameTimes[32];
    static int FrameTimesIndex;
    
    //Compute the average of the last 32 frame times
    CpuFrameTimes[FrameTimesIndex] = CpuSecondsElapsed;
    GpuFrameTimes[FrameTimesIndex] = GpuSecondsElapsed;
    FrameTimesIndex = (FrameTimesIndex + 1) % ArrayCount(CpuFrameTimes);
    
    f32 AverageCpuSecondsElapsed = 0.0f;
    f32 AverageGpuSecondsElapsed = 0.0f;
    for(u32 Index = 0; Index < ArrayCount(CpuFrameTimes); Index++)
    {
        AverageCpuSecondsElapsed += CpuFrameTimes[Index];
        AverageGpuSecondsElapsed += GpuFrameTimes[Index];
    }
    AverageCpuSecondsElapsed /= ArrayCount(CpuFrameTimes);
    AverageGpuSecondsElapsed /= ArrayCount(GpuFrameTimes);
    
    static bool Show = true;
    ImGui::SetNextWindowPos(Position, ImGuiCond_Always, vec2(0, 0));
    ImGui::SetNextWindowBgAlpha(0.5f); // Transparent background
    ImGuiWindowFlags Flags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize   |
        ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav |
        ImGuiWindowFlags_NoDocking;
    if (ImGui::Begin("Debug Data", &Show, Flags))
    {
        ImGui::Text("FPS: %.3f", 1.0f / AverageCpuSecondsElapsed);
        ImGui::Text("CPU Frame Time: %.3fms", AverageCpuSecondsElapsed * 1000.0f);
        ImGui::Text("GPU Frame Time: %.3fms", AverageGpuSecondsElapsed * 1000.0f);
        ImGui::End();
    }
}

internal void
DrawCubemapToTexture(d3d11_state* D3D11, ID3D11ShaderResourceView* Cubemap, u32 MipLevel,
                     d3d11_render_target* Target, d3d11_depth_buffer* Depth, 
                     vec3 From, vec3 To, f32 FOV, b32 Linear, mat4 Rotation, bool Sphere = false)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    Context->ClearRenderTargetView(Target->RenderTarget, vec4(0.0f).e);
    Context->ClearDepthStencilView(Depth->DepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    D3D11_VIEWPORT Viewport = {};
    Viewport.Width = (float)Target->Width;
    Viewport.Height = (float)Target->Height;
    Viewport.MaxDepth = 1.0f;
    
    Context->ClearState();
    Context->OMSetRenderTargets(1, &Target->RenderTarget, Depth->DepthView);
    Context->OMSetDepthStencilState(D3D11->Common.LessDepthStencilState, 0);
    Context->RSSetState(D3D11->Common.SolidRasterStateNoCull);
    Context->RSSetViewports(1, &Viewport);
    
    u32 Strides[] = {sizeof(vec3)};
    u32 Offsets[] = {0};
    if (!Sphere)
    {
        Context->IASetVertexBuffers(0, 1, &D3D11->Common.CubemapVertexBuffer, Strides, Offsets);
        Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
    else
    {
        Context->IASetVertexBuffers(0, 1, &D3D11->Common.SphereVertexBuffer, Strides, Offsets);
        Context->IASetIndexBuffer(D3D11->Common.SphereIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    }
    Context->IASetInputLayout(D3D11->Common.CubemapLayout);
    
    Context->VSSetShader(D3D11->Common.CubemapVertexShader, 0, 0);
    Context->PSSetShader(D3D11->Common.CubemapPixelShader, 0, 0);
    Context->PSSetShaderResources(0, 1, &Cubemap);
    Context->PSSetSamplers(0, 1, Linear ? &D3D11->Common.LinearSamplerState : &D3D11->Common.PointSamplerState);
    
    ID3D11Buffer* VertexConstantsBuffer = D3D11->Common.CubemapVertexConstantsBuffer;
    d3d11_cubemap_vertex_constants VertexConstants = {};
    f32 AspectRatio = (f32)Target->Width / (f32)Target->Height;
    VertexConstants.Projection = Mat4Perspective(FOV, 0.1f, 10.0f, AspectRatio);
    VertexConstants.View = Mat4LookAt(From, To, vec3(0.0f, 0.0f, 1.0f));
    VertexConstants.Model = Rotation;
    D3D11_FillConstantBuffers(Context, VertexConstantsBuffer, &VertexConstants, sizeof(VertexConstants));
    
    ID3D11Buffer* PixelConstantsBuffer = D3D11->Common.CubemapPixelConstantsBuffer;
    d3d11_cubemap_pixel_constants PixelConstants = {};
    PixelConstants.MipLevel = MipLevel;
    D3D11_FillConstantBuffers(Context, PixelConstantsBuffer, &PixelConstants, sizeof(PixelConstants));
    
    Context->VSSetConstantBuffers(0, 1, &VertexConstantsBuffer);
    Context->PSSetConstantBuffers(0, 1, &PixelConstantsBuffer);
    if (!Sphere)
    {
        Context->Draw(36, 0);
    }
    else
    {
        Context->DrawIndexed(D3D11->Common.SphereIndicesCount, 0, 0);
    }
}

internal void
DrawTexture3DSliceToTexture(d3d11_state* D3D11, ID3D11ShaderResourceView* Texture, f32 w,
                            d3d11_render_target* Target, bool Linear)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    Context->ClearState();
    
    D3D11_SetViewport(Context, Target->Width, Target->Height);
    Context->OMSetRenderTargets(1, &Target->RenderTarget, 0);
    Context->RSSetState(D3D11->Common.SolidRasterStateNoCull);
    Context->PSSetShader(D3D11->Common.Texture3DSlicePixelShader, 0, 0);
    Context->PSSetShaderResources(0, 1, &Texture);
    Context->PSSetSamplers(0, 1, Linear ? &D3D11->Common.LinearSamplerState : &D3D11->Common.PointSamplerState);
    
    ID3D11Buffer* Constants = D3D11->Common.Texture3DSliceConstantsBuffer;
    D3D11_FillConstantBuffers(Context, Constants, &w, sizeof(w));
    Context->PSSetConstantBuffers(0, 1, &Constants);
    
    D3D11_DrawFrame(D3D11);
}

internal void
DrawSHToTexture(d3d11_state* D3D11, d3d11_render_target* Target, d3d11_depth_buffer* Depth, 
                vec3 From, vec3 To, float FOV, mat4 Rotation, bool Basis, vec4* L)
{
    ID3D11DeviceContext* Context = D3D11->Context;
    Context->ClearRenderTargetView(Target->RenderTarget, vec4(0.0f).e);
    Context->ClearDepthStencilView(Depth->DepthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    D3D11_VIEWPORT Viewport = {};
    Viewport.Width = (float)Target->Width;
    Viewport.Height = (float)Target->Height;
    Viewport.MaxDepth = 1.0f;
    
    Context->ClearState();
    Context->OMSetRenderTargets(1, &Target->RenderTarget, Depth->DepthView);
    Context->OMSetDepthStencilState(D3D11->Common.LessDepthStencilState, 0);
    Context->RSSetState(D3D11->Common.SolidRasterStateNoCull);
    Context->RSSetViewports(1, &Viewport);
    
    u32 Strides[] = {sizeof(vec3)};
    u32 Offsets[] = {0};
    Context->IASetVertexBuffers(0, 1, &D3D11->Common.SphereVertexBuffer, Strides, Offsets);
    Context->IASetIndexBuffer(D3D11->Common.SphereIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    Context->IASetInputLayout(D3D11->Common.CubemapLayout);
    
    if(Basis)
    {
        Context->VSSetShader(D3D11->SH.TestVertexShader, 0, 0);
        Context->PSSetShader(D3D11->SH.TestPixelShader, 0, 0);
    }
    else
    {
        Context->VSSetShader(D3D11->SH.LightVertexShader, 0, 0);
        Context->PSSetShader(D3D11->SH.LightPixelShader, 0, 0);
        
        ID3D11Buffer* PixelConstantsBuffer = D3D11->SH.PixelConstantsBuffer;
        D3D11_FillConstantBuffers(Context, PixelConstantsBuffer, L, sizeof(vec4) * 9);
        Context->PSSetConstantBuffers(0, 1, &PixelConstantsBuffer);
    }
    
    ID3D11Buffer* VertexConstantsBuffer = D3D11->Common.CubemapVertexConstantsBuffer;
    d3d11_cubemap_vertex_constants VertexConstants = {};
    f32 AspectRatio = (f32)Target->Width / (f32)Target->Height;
    VertexConstants.Projection = Mat4Perspective(FOV, 0.1f, 10.0f, AspectRatio);
    VertexConstants.View = Mat4LookAt(From, To, vec3(0.0f, 0.0f, 1.0f));
    VertexConstants.Model = Rotation;
    D3D11_FillConstantBuffers(Context, VertexConstantsBuffer, &VertexConstants, sizeof(VertexConstants));
    
    Context->VSSetConstantBuffers(0, 1, &VertexConstantsBuffer);
    Context->DrawIndexed(D3D11->Common.SphereIndicesCount, 0, 0);
}

vec2 FitToAspectRatio(vec2 Size, f32 AspectRatio)
{
    f32 CurrentRatio = Size.x / Size.y;
    if(CurrentRatio > AspectRatio) {
        return vec2(Size.y * AspectRatio, Size.y);
    } else {
        return vec2(Size.x, Size.x / AspectRatio);
    }
}

internal void
DrawTextureViewer(d3d11_state* D3D11)
{
    // left
    static u32 Selected = 0;
    ImGui::BeginChild("Texture Selector", ImVec2(150, 0), true);
    for (u32 i = 0; i < InspectorData.TrackedTexturesCount; i++)
    {
        tracked_texture* Track = &InspectorData.TrackedTextures[i];
        char label[128];
        sprintf(label, "MyObject %d", i);
        if (ImGui::Selectable(Track->Name, Selected == i))
            Selected = i;
    }
    ImGui::EndChild();
    
    ImGui::SameLine();
    
    ImGui::BeginChild("Texture Viewer");
    if(InspectorData.TrackedTexturesCount > 0)
    {
        tracked_texture* Track = &InspectorData.TrackedTextures[Selected];
        
        //Share angles for easier compare
        static f32 AngleZ;
        static f32 AngleY;
        
        
        if(Track->Kind == TRACKED_TEXTURE_2D)
        {
            ImGui::Text("%s [%dx%d]", Track->Name, Track->Size.x, Track->Size.y);
            
            ImVec2 AvailableSize = ImGui::GetContentRegionAvail();
            f32 AspectRatio = (f32)Track->Size.x / (f32)Track->Size.y;
            ImVec2 ImageSize = FitToAspectRatio(AvailableSize, AspectRatio);
            
            //Holy typesafety, hope the C++ committee doesn't see this (but it works)
            ImGui::Image(Track->Texture, ImageSize);
        }
        else if(Track->Kind == TRACKED_TEXTURE_3D)
        {
            ImGui::Text("%s [%dx%dx%d]", Track->Name, Track->Size.x, Track->Size.y, Track->Depth);
            
            static bool Linear = true;
            static s32 Slice = 0;
            ImGui::Checkbox("Linear", &Linear);
            ImGui::SameLine();
            
            ImGui::Text("Slice:");
            ImGui::SameLine();
            // Arrow buttons with Repeater
            ImGui::DragInt("Slice:", &Slice, 0.5f, 0, Track->Depth - 1);
            
            ImVec2 AvailableSize = ImGui::GetContentRegionAvail();
            f32 AspectRatio = (f32)Track->Size.x / (f32)Track->Size.y;
            ImVec2 ImageSize = FitToAspectRatio(AvailableSize, AspectRatio);
            
            static d3d11_render_target Target;
            if((u32)AvailableSize.x > 0 && (u32)AvailableSize.y > 0)
            {
                if(Target.Width != AvailableSize.x || Target.Height != AvailableSize.y)
                {
                    if(Target.Width > 0 && Target.Height > 0)
                    {
                        D3D11_FreeRenderTarget(&Target);
                    }
                    Target = D3D11_CreateRenderTargetLDR(D3D11->Device, (u32)AvailableSize.x, (u32)AvailableSize.y, false, false);
                }
                
                f32 w = (f32)(Slice + 0.5f) / (f32)Track->Depth;
                DrawTexture3DSliceToTexture(D3D11, (ID3D11ShaderResourceView*)Track->Texture, w, &Target, Linear);
            }
            
            
            ImGui::Image(Target.ResourceView, ImageSize);
            
        }
        else if(Track->Kind == TRACKED_TEXTURE_CUBEMAP)
        {
            ImGui::Text("%s [%dx%d]", Track->Name, Track->Size.x, Track->Size.y);
            
            static d3d11_render_target Target;
            static d3d11_depth_buffer Depth;
            static f32 FOV = 60.0f;
            static bool Inside;
            static int MipLevel = 0;
            static bool Linear = true;
            static bool Sphere = true;
            
            ImGui::SliderFloat("Rotation Z", &AngleZ, -360.0f, 360.0f);
            ImGui::SameLine();
            if(ImGui::Button("Reset##Z"))
            {
                AngleZ = 0.0f;
            }
            ImGui::SliderFloat("Rotation Y", &AngleY, -180.0f, 180.0f);
            ImGui::SameLine();
            if(ImGui::Button("Reset##Y"))
            {
                AngleY = 0.0f;
            }
            
            ImGui::Checkbox("Inside", &Inside);
            ImGui::SameLine();
            
            ImGui::Checkbox("Linear", &Linear);
            ImGui::SameLine();
            
            ImGui::Checkbox("Sphere", &Sphere);
            ImGui::SameLine();
            
            ImGui::Text("Mip Level:");
            ImGui::SameLine();
            // Arrow buttons with Repeater
            float Spacing = ImGui::GetStyle().ItemInnerSpacing.x;
            if (ImGui::ArrowButton("##left", ImGuiDir_Left)) 
            { 
                if(MipLevel > 0)
                    MipLevel--; 
            }
            ImGui::SameLine(0.0f, Spacing);
            if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
                //                if(MipLevel < 4)
                MipLevel++;
            }
            ImGui::SameLine();
            ImGui::Text("%d", MipLevel);
            
            
            ImVec2 AvailableSize = ImGui::GetContentRegionAvail();
            if((u32)AvailableSize.x > 0 && (u32)AvailableSize.y > 0)
            {
                
                if(Target.Width != AvailableSize.x || Target.Height != AvailableSize.y)
                {
                    if(Target.Width > 0 && Target.Height > 0)
                    {
                        D3D11_FreeRenderTarget(&Target);
                        D3D11_FreeDepthBuffer(&Depth);
                    }
                    Target = D3D11_CreateRenderTargetLDR(D3D11->Device, (u32)AvailableSize.x, (u32)AvailableSize.y, true, false);
                    Depth = D3D11_CreateDepthBuffer(D3D11->Device, (u32)AvailableSize.x, (u32)AvailableSize.y);
                }
                
                vec3 From;
                vec3 To;
                if(Inside)
                {
                    From = vec3(0, 0, 0);
                    To = vec3(1, 1, 0);
                }
                else
                {
                    From = vec3(1, 1, 1);
                    To = vec3(0, 0, 0);
                }
                
                mat4 Rotation = Mat4Rotate(AngleZ, vec3(0.0f, 0.0f, 1.0f)) * Mat4Rotate(AngleY, vec3(0.0f, 1.0f, 0.0f));
                DrawCubemapToTexture(D3D11, (ID3D11ShaderResourceView*)Track->Texture, MipLevel,
                                     &Target, &Depth, From, To, FOV, Linear, Rotation, Sphere);
                
                ImGui::Image(Target.ResourceView, AvailableSize);
            }
        }
        else if(Track->Kind == TRACKED_SH_MAP)
        {
            ImGui::Text("%s", Track->Name);
            static d3d11_render_target Target;
            static d3d11_depth_buffer Depth;
            static f32 FOV = 60.0f;
            static bool Inside;
            static bool Basis;
            
            ImGui::SliderFloat("Rotation Z", &AngleZ, -360.0f, 360.0f);
            ImGui::SameLine();
            if(ImGui::Button("Reset##Z"))
            {
                AngleZ = 0.0f;
            }
            ImGui::SliderFloat("Rotation Y", &AngleY, -180.0f, 180.0f);
            ImGui::SameLine();
            if(ImGui::Button("Reset##Y"))
            {
                AngleY = 0.0f;
            }
            
            ImGui::Checkbox("Inside", &Inside);
            ImGui::SameLine();
            
            ImGui::Checkbox("Basis", &Basis);
            
            ImVec2 AvailableSize = ImGui::GetContentRegionAvail();
            if((u32)AvailableSize.x > 0 && (u32)AvailableSize.y > 0)
            {
                
                if(Target.Width != AvailableSize.x || Target.Height != AvailableSize.y)
                {
                    if(Target.Width > 0 && Target.Height > 0)
                    {
                        D3D11_FreeRenderTarget(&Target);
                        D3D11_FreeDepthBuffer(&Depth);
                    }
                    Target = D3D11_CreateRenderTargetLDR(D3D11->Device, (u32)AvailableSize.x, (u32)AvailableSize.y, true, false);
                    Depth = D3D11_CreateDepthBuffer(D3D11->Device, (u32)AvailableSize.x, (u32)AvailableSize.y);
                }
                
                vec3 From;
                vec3 To;
                if(Inside)
                {
                    From = vec3(0, 0, 0);
                    To = vec3(1, 1, 0);
                }
                else
                {
                    From = vec3(1, 1, 1);
                    To = vec3(0, 0, 0);
                }
                
                mat4 Rotation = Mat4Rotate(AngleZ, vec3(0.0f, 0.0f, 1.0f)) * Mat4Rotate(AngleY, vec3(0.0f, 1.0f, 0.0f));
                DrawSHToTexture(D3D11, &Target, &Depth, From, To, FOV, Rotation, Basis, InspectorData.L);
                
                ImGui::Image(Target.ResourceView, AvailableSize);
            }
        }
    }
    ImGui::EndChild();
}

internal void
DrawSceneInspector(scene* Scene)
{
    if(ImGui::CollapsingHeader("Meshes"))
    {
        //Compute mesh draw transforms from position
        for(u32 i = 0; i < Scene->MeshesCount; i++)
        {
            mesh* Mesh = Scene->Meshes + i;
            
            if(ImGui::TreeNode(Mesh, Mesh->Name))
            {
                vec3 InitialPos = Mesh->Position;
                vec3 InitialRot = Mesh->Rotation;
                vec3 InitialScale = Mesh->Scale;
                
                ImGui::DragFloat3("Position", Mesh->Position.e, 0.01f, -1000.0f, 1000.0f);
                ImGui::DragFloat3("Rotation", Mesh->Rotation.e, 0.1f, -1000.0f, 1000.0f);
                ImGui::DragFloat3("Scale",    Mesh->Scale.e,    0.01f, 0.0f, 1000.0f);
                
                if(InitialPos != Mesh->Position ||
                   InitialRot != Mesh->Rotation ||
                   InitialScale != Mesh->Scale)
                {
                    Mesh->AABBDirty = true;
                }
                
                material* Mat = Scene->Materials + Mesh->MaterialIndex;
                if(ImGui::BeginCombo("Material", Mat->Name, 0))
                {
                    for(u32 j = 0; j < Scene->MaterialsCount; j++)
                    {
                        bool Selected = (Mat == Scene->Materials + j);
                        if(ImGui::Selectable(Scene->Materials[j].Name, Selected))
                            Mat = Scene->Materials + j;
                        if(Selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                Mesh->MaterialIndex = (u32)(Mat - Scene->Materials);
                
                ImGui::TreePop();
            }
        }
    }
    
    if(ImGui::CollapsingHeader("Point Lights"))
    {
        //Compute mesh draw transforms from position
        for(u32 i = 0; i < Scene->PointLightsCount; i++)
        {
            point_light* Light = Scene->PointLights + i;
            if(ImGui::TreeNode(Light, "Light %d", i))
            {
                ImGui::DragFloat3("Position", Light->Position.e, 0.01f, -1000.0f, 1000.0f);
                ImGui::DragFloat("Radius", &Light->Radius, 0.01f, 0.0f, 1000.0f);
                ImGui::DragFloat3("Color", Light->Color.e, 0.1f, 0.0f, 100.0f);
                
                ImGui::Text("Shadow Frustum:");
                ImGui::SameLine();
                ImGui::Checkbox("Debug draw light position", &Light->DebugDrawLightPosition);
                ImGui::Checkbox("Debug draw frustum", &Light->DebugDrawFrustum);
                ImGui::InputInt("Face index", &Light->DebugFrustumFace);
                Light->DebugFrustumFace = ClampS32(Light->DebugFrustumFace, 0, 5);
                
                ImGui::TreePop();
            }
        }
        
    }
    
    if(ImGui::CollapsingHeader("Directional Lights"))
    {
        //Compute mesh draw transforms from position
        for(u32 i = 0; i < Scene->DirectionalLightsCount; i++)
        {
            directional_light* Light = Scene->DirectionalLights + i;
            if(ImGui::TreeNode(Light, "Light %d", i))
            {
                ImGui::DragFloat3("Direction", Light->Direction.e, 0.01f, -1.0f, 1.0f);
                if(LengthSquared(Light->Direction) > 1.0e-8f)
                    Light->Direction = Normalize(Light->Direction);
                
                //TODO: Color picker
                
                ImGui::Text("Shadow Frustum:");
                ImGui::SameLine();
                ImGui::Checkbox("Debug draw", &Light->DebugDrawFrustum);
                ImGui::DragFloat3("Position", Light->ShadowPosition.e, 0.01f, -1000.0f, 1000.0f);
                ImGui::DragFloat2("Size", Light->ShadowFrustumSize.e, 0.01f, 0, 1000.0f);
                ImGui::DragFloat("Near", &Light->ShadowFrustumNear, 0.01f, 0, Light->ShadowFrustumFar);
                ImGui::DragFloat("Far", &Light->ShadowFrustumFar, 0.01f, Light->ShadowFrustumNear, 1000.0f);
                
                ImGui::TreePop();
            }
        }
        
    }
    
    if(ImGui::CollapsingHeader("Atmosphere"))
    {
        ImGui::DragFloat("Sun Illuminance scale", &Scene->SunIlluminanceScale, 0.1f, 0.0f, 100.0f);
        ImGui::DragFloat3("Sun Illuminance color", Scene->SunIlluminanceColor.e, 0.1f, 0.0f, 1.0f);
        
        static float SunPhi, SunTheta;
        
        SunPhi = PhiFromDirectionDeg(Scene->SunDirection);
        SunTheta = ThetaFromDirectionDeg(Scene->SunDirection);
        
        if(SunPhi < 0.0f) SunPhi += 360.0f;
        if(SunTheta < 0.0f) SunTheta += 360.0f;
        
        ImGui::DragFloat("Sun theta", &SunTheta, 0.1f, 0, 180.0f);
        ImGui::DragFloat("Sun phi", &SunPhi, 0.1f, 0, 360.0f);
        
        Scene->SunDirection = DirectionFromThetaPhiDeg(SunTheta, SunPhi);
        
        s32 TransmittanceTextureSize = TRANSMITTANCE_TEXTURE_WIDTH * TRANSMITTANCE_TEXTURE_HEIGHT * 16;
        s32 IrradianceTextureSize = IRRADIANCE_TEXTURE_WIDTH * IRRADIANCE_TEXTURE_HEIGHT * 16;
        s32 ScatteringTextureSize = SCATTERING_TEXTURE_WIDTH * SCATTERING_TEXTURE_HEIGHT * SCATTERING_TEXTURE_DEPTH * 16;
        s32 PrecomputeSize = IrradianceTextureSize + 3 * ScatteringTextureSize;
        s32 RuntimeSize = TransmittanceTextureSize + IrradianceTextureSize + ScatteringTextureSize;
        ImGui::Text("Precompute size: %.3fMB", (f32)PrecomputeSize / Megabytes(1));
        ImGui::Text("Runtime size: %.3fMB", (f32)RuntimeSize / Megabytes(1));
        ImGui::Text("Total size: %.3fMB", (f32)(PrecomputeSize + RuntimeSize) / Megabytes(1));
    }
}


internal b32
IsPointInRect(ImVec2 P, ImVec2 A, ImVec2 B)
{
    b32 Result = P.x >= A.x && P.y >= A.y && P.x < B.x && P.y <= B.y;
    
    return Result;
}

internal void 
UpdateGpuFrameSlider(d3d11_state* D3D11)
{
    if(!GpuFrameSlider.Paused) 
    {
        d3d11_profiler* Profiler = &D3D11->Profiler;
        
        //Store frame data in the static array
        GpuFrameSlider.Frames[GpuFrameSlider.CurrentFrameIndex].Time = Profiler->FrameTime;
        for(u32 i = 0; i < ArrayCount(Profiler->AreaTime); i++) {
            GpuFrameSlider.Frames[GpuFrameSlider.CurrentFrameIndex].AreaTime[i] = Profiler->AreaTime[i];
        }
        
        GpuFrameSlider.CurrentFrameIndex = (GpuFrameSlider.CurrentFrameIndex + 1) % ArrayCount(GpuFrameSlider.Frames);
    }
}

internal void
DrawProfiler(d3d11_profiler* Profiler)
{
    if(!Profiler->FrameIsValid) {
        return;
    }
    
    if(GpuFrameSlider.Paused)
    {
        if(ImGui::Button("Resume Profiling"))
        {
            GpuFrameSlider.Paused = false;
        }
    }
    else
    {
        if(ImGui::Button("Pause Profiling"))
        {
            GpuFrameSlider.Paused = true;
        }
    }
    
    //Draw the array
    ImVec2 MousePos = ImGui::GetMousePos();
    ImDrawList* DrawList = ImGui::GetWindowDrawList();
    ImVec2 Begin = ImGui::GetCursorScreenPos();
    ImVec2 AvailableSize = ImGui::GetContentRegionAvail();
    
    u32 FramesCount = ArrayCount(GpuFrameSlider.Frames);
    f32 Width = AvailableSize.x;
    f32 BarWidth = AvailableSize.x / FramesCount;
    
    b32 TooltipBegan = false;
    for(u32 FrameIndex = 0; FrameIndex < FramesCount; FrameIndex++)
    {
        f32 CurrentX = Begin.x + BarWidth * FrameIndex;
        f32 CurrentY = Begin.y;
        f32 FrameTime = GpuFrameSlider.Frames[FrameIndex].Time;
        
        ImVec2 FrameBoxA = ImVec2(CurrentX, CurrentY);
        ImVec2 FrameBoxB = FrameBoxA + ImVec2(1.0f + BarWidth, AvailableSize.y);
        b32 TooltipDone = true;
        if(!TooltipBegan && IsPointInRect(MousePos, FrameBoxA, FrameBoxB))
        {
            ImGui::BeginTooltip();
            ImGui::Text("Total Frame Time: %.3fms", FrameTime * 1000.0f);
            ImGui::Separator();
            TooltipDone = false;
            TooltipBegan = true;
        }
        
        ImVec2 FrameMarkerA = ImVec2(CurrentX, CurrentY);
        ImVec2 FrameMarkerB = ImVec2(CurrentX, CurrentY + AvailableSize.y);
        if(FrameIndex == GpuFrameSlider.CurrentFrameIndex || FrameIndex == GpuFrameSlider.CurrentFrameIndex - 1)
        {
            u32 MarkerColor = RGB(0, 255, 0);
            DrawList->AddLine(FrameMarkerA, FrameMarkerB, MarkerColor, 1.0f);
        }
        
        f32 UntrackedTime = FrameTime;
        for(u32 AreaIndex = 0; AreaIndex < D3D11_PROFILE_AREAS_COUNT; AreaIndex++)
        {
            char* Name = D3D11ProfilerAreaNames[AreaIndex];
            f32 AreaTime = GpuFrameSlider.Frames[FrameIndex].AreaTime[AreaIndex];
            UntrackedTime -= AreaTime;
            f32 AreaHeight = AreaTime / FrameTime * AvailableSize.y;
            
            ImVec2 A = ImVec2(CurrentX + 1.0f, CurrentY);
            ImVec2 B = A + ImVec2(BarWidth, AreaHeight);
            
            if(IsPointInRect(MousePos, A, B) && !TooltipDone)
            {
                ImGui::Text("%s: %.3fms", Name, AreaTime * 1000.0f);
                ImGui::EndTooltip();
                TooltipDone = true;
            }
            
            B.x -= 1.0f; //Make the bar slightly smaller
            u32 Color = D3D11ProfilerAreaColors[AreaIndex];
            DrawList->AddRectFilled(A, B, Color, 0.0f, 0);
            
            CurrentY += AreaHeight;
        }
        
        ImVec2 A = ImVec2(CurrentX + 1.0f, CurrentY);
        ImVec2 B = ImVec2(CurrentX + BarWidth - 1.0f, Begin.y + AvailableSize.y);
        if(!TooltipDone)
        {
            ImGui::Text("Untracked Time: %.3fms", UntrackedTime * 1000.0f);
            ImGui::EndTooltip();
            TooltipDone = true;
        }
        
        if(UntrackedTime > 0)
        {
            u32 Color = RGB(0,0,0);
            DrawList->AddRectFilled(A, B, Color, 0.0f, 0);
        }
    }    
}

internal void
DrawRenderSettings()
{
    ImGui::Checkbox("VSync", &InspectorData.VSyncEnabled);
    
    ImGui::Checkbox("Depth prepass", &InspectorData.DepthPrepass);
    
    ImGui::Checkbox("Camera frustum culling", &InspectorData.FrustumCulling);
    ImGui::Text("Objects drawn: %d", InspectorData.ObjectsDrawn);
    
    ImGui::Checkbox("Shadow cubemap frustum culling", &InspectorData.ShadowCubemapFrustum);
    ImGui::Checkbox("Frustum frustum culling", &InspectorData.FrustumFrustumCulling);
    ImGui::Text("Objects drawn on cubemap: %d", InspectorData.ObjectsDrawnOnCubemap);
    
    ImGui::InputInt("Plane index", &InspectorData.PlaneIndex);
    InspectorData.PlaneIndex = ClampS32(InspectorData.PlaneIndex, 0, 5);    
    
    ImGui::DragFloat("Aerial perspective scale", &InspectorData.AerialPerspectiveScale, 0.1f, 1.0f, 1000.0f);
    
    for(u32 i = 0; i < ArrayCount(InspectorData.L); i++)
    {
        ImGui::PushID(i);
        ImGui::DragFloat3("L", &InspectorData.L[i].x, 0.01f, -1, 1);
        ImGui::PopID();
    }
}

internal void
DrawAssets(asset_table Table)
{
    for(u32 Index = 0; Index < Table.Count; Index++)
    {
        asset_table_entry* Entry = &Table.Entries[Index];
        ImGui::Text("%s - %s | %s", Entry->Name, Entry->Tag, AssetTypeToName[Entry->Type]);
    }
}

internal void
DrawEditor(d3d11_state* D3D11, scene* Scene, asset_table AssetTable, float SecondsElapsed)
{
    //DockSpace
    ImGuiWindowFlags DockSpaceWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    ImGuiViewport* Viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(Viewport->GetWorkPos());
    ImGui::SetNextWindowSize(Viewport->GetWorkSize());
    ImGui::SetNextWindowViewport(Viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    DockSpaceWindowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    DockSpaceWindowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    ImGui::Begin("DockSpace Demo", 0, DockSpaceWindowFlags);
    ImGui::PopStyleVar(3);
    
    // DockSpace
    ImGuiIO& io = ImGui::GetIO();
    ImGuiID DockSpaceId = ImGui::GetID("MyDockSpace");
    ImGuiDockNodeFlags DockSpaceFlags = ImGuiDockNodeFlags_None;
    ImGui::DockSpace(DockSpaceId, ImVec2(0.0f, 0.0f), DockSpaceFlags);
    
    if(ImGui::BeginMenuBar())
    {
        if(ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("Exit"))
            {
                Win32.WindowQuit = true;
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
    ImGui::End();
    
    
    
    //Windows
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoCollapse;
    if(ImGui::Begin("Render Settings", 0, WindowFlags))
    {
        DrawRenderSettings();
    }
    ImGui::End();
    
    if(ImGui::Begin("Textures", 0, WindowFlags))
    {
        DrawTextureViewer(D3D11);
    }
    ImGui::End();
    
    if(ImGui::Begin("GPU Profiler", 0, WindowFlags))
    {
        DrawProfiler(&D3D11->Profiler);
    }
    ImGui::End();
    
    if(ImGui::Begin("Startup Timings", 0, WindowFlags))
    {
        u64 Begin = StartupTimestamps.Begin;
        for(u32 i = 0; i < STARTUP_ACTIONS_COUNT; i++)
        {
            u64 End = StartupTimestamps.Timestamps[i];
            f32 ms = Win32_GetSecondsElapsed(Begin, End) * 1000.0f;
            ImGui::Text("%s: %.3fms", StartupActionNames[i], ms);
            Begin = End;
        }
        ImGui::Separator();
        ImGui::Text("Total startup time: %.3fs", Win32_GetSecondsElapsed(StartupTimestamps.Begin, Begin));
        
    }
    ImGui::End();
    
    if(ImGui::Begin("GPU Timings", 0, WindowFlags))
    {
        for(u32 i = 0; i < ArrayCount(D3D11->Profiler.AreaTime); i++)
        {
            ImGui::Text("%s: %.3fms", D3D11ProfilerAreaNames[i], D3D11->Profiler.AreaTime[i] * 1000.0f);
        }
    }
    ImGui::End();
    
    if(ImGui::Begin("Assets", 0, WindowFlags))
    {
        DrawAssets(AssetTable);
    }
    ImGui::End();
    
    if(ImGui::Begin("Scene", 0, WindowFlags))
    {
        DrawSceneInspector(Scene);
    }
    ImGui::End();
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    ImGuiWindowFlags ViewportWindowFlags = ImGuiWindowFlags_NoCollapse;
    bool ShowStats = false;
    ivec2 StatsPos = ivec2(0, 0);
    if(ImGui::Begin("Viewport", 0, ViewportWindowFlags))
    {
        ImVec2 AvailableSize = ImGui::GetContentRegionAvail();
        
        if(AvailableSize.x > 200 && AvailableSize.y > 100)
        {
            ShowStats = true;
            ivec2 ScreenPos = ivec2(ImGui::GetCursorScreenPos());
            StatsPos = ScreenPos + ivec2(10, 10);
        }
        
        vec2 FitSize = FitToAspectRatio(AvailableSize, 16.0f / 9.0f);
        
        ivec2 ViewportSize = ivec2(FitSize);
        if(ViewportSize.x > 0 && ViewportSize.y > 0 &&
           ViewportSize != ivec2(D3D11->DefaultRenderTarget.Width, D3D11->DefaultRenderTarget.Height))
        {
            D3D11_ResizeDefaultRenderTargets(D3D11, ViewportSize);
        }
        
        ivec2 HalfExtraSize = ivec2((vec2(AvailableSize) - vec2(FitSize)) / 2.0f);
        ivec2 BeginPos = ivec2(ImGui::GetCursorPos());
        ImGui::SetCursorPos(vec2(HalfExtraSize + BeginPos));
        
        ImGui::Image(D3D11->DefaultRenderTarget.ResourceView, (vec2)ViewportSize);
    }
    
    ImGui::End();
    ImGui::PopStyleVar();
    
    if(ShowStats)
    {
        DrawStats(vec2(StatsPos), SecondsElapsed, D3D11->Profiler.FrameTime);
    }
    
}
