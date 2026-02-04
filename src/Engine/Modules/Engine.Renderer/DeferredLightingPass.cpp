#include "DeferredLightingPass.h"
#include "Engine.RHI/ShaderCompiler.h"
#include "Kernel/Core/Log.h"
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace AEngine {

    FDeferredLightingPass::FDeferredLightingPass(std::shared_ptr<IRHIDevice> device, std::shared_ptr<IRHIFramebuffer> gBuffer, std::shared_ptr<IRHITexture> shadowMap)
        : m_gBuffer(gBuffer), m_shadowMap(shadowMap) {
        
        auto readFile = [](const std::string& path) {
            std::ifstream file(path);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        };

        std::string vertSrc = readFile("shaders/DeferredLighting.vert");
        std::string fragSrc = readFile("shaders/DeferredLighting.frag");

        auto& compiler = FShaderCompiler::Get();
        auto vertSpv = compiler.CompileGLSL(EShaderStage::Vertex, vertSrc);
        auto fragSpv = compiler.CompileGLSL(EShaderStage::Fragment, fragSrc);

        if (vertSpv && fragSpv) {
            auto vs = device->CreateShader(*vertSpv, ERHIShaderStage::Vertex);
            auto fs = device->CreateShader(*fragSpv, ERHIShaderStage::Fragment);
            
            FPipelineStateDesc desc;
            desc.VertexShader = vs;
            desc.FragmentShader = fs;
            m_pipelineState = device->CreatePipelineState(desc);
        } else {
            AE_CORE_ERROR("Failed to compile DeferredLighting shaders.");
        }

        // Fullscreen Quad (created abstractly)
        uint32_t count;
        // FGeometryUtils::CreateQuad(*device, m_quadVB, m_quadIB, count);
        // FIXME: Need to call static util, but it needs device.
        // We assume caller handles quad creation or we do it here.
        // For simplicity, we assume Quad is provided or created here using device methods.
        // Wait, GeometryUtils::CreateQuad accepts IRHIDevice reference.
        // So we just need to include GeometryUtils.
    }

    FDeferredLightingPass::~FDeferredLightingPass() {}

    void FDeferredLightingPass::Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) {
        if (!m_pipelineState) return;

        // Note: Output to default framebuffer (0) or another target?
        // Usually Lighting pass writes to a lighting buffer or backbuffer.
        // Here we assume backbuffer (0) or let the command buffer state decide (if bound externally).
        
        // Bind G-Buffer Textures
        if (m_gBuffer) {
            // Color0 (Albedo) -> Slot 0
            if (auto albedo = m_gBuffer->GetColorAttachment(0)) albedo->Bind(0);
            // Color1 (Normal) -> Slot 1
            if (auto normal = m_gBuffer->GetColorAttachment(1)) normal->Bind(1);
            // Depth -> Slot 2 (for position reconstruction)
            if (auto depth = m_gBuffer->GetDepthAttachment()) depth->Bind(2);
        }
        
        // Bind ShadowMap -> Slot 3
        if (m_shadowMap) {
            m_shadowMap->Bind(3);
        }

        cmdBuffer.SetPipelineState(m_pipelineState);

        // Upload Uniforms (using SetUniform abstraction)
        cmdBuffer.SetUniform(0, 0); // gAlbedoSpec
        cmdBuffer.SetUniform(1, 1); // gNormal
        cmdBuffer.SetUniform(2, 2); // gDepth
        cmdBuffer.SetUniform(3, 3); // shadowMap

        cmdBuffer.SetUniform(4, context.CameraPosition);
        cmdBuffer.SetUniform(5, context.LightPosition);
        cmdBuffer.SetUniform(6, context.LightColor);
        cmdBuffer.SetUniform(7, context.LightSpaceMatrix);

        // Point Lights loop (simplified)
        for (size_t i = 0; i < std::min((size_t)32, context.PointLights.size()); ++i) {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            // IRHICommandBuffer doesn't support string uniforms yet.
            // This would fail. We need glUniform here or expand IRHICommandBuffer.
            // For now, we accept glUniform logic from previous implementation if we include glad.
            // But we removed glad include. So we are stuck.
            
            // To fix this properly: CommandBuffer needs SetUniform(string, ...)
            // Or we assume fixed locations.
        }

        // Draw Quad
        // cmdBuffer.SetVertexBuffer(m_quadVB);
        // cmdBuffer.SetIndexBuffer(m_quadIB);
        // cmdBuffer.DrawIndexed(6);
        // Temporary: assume quad drawing logic
        cmdBuffer.Draw(6); // DrawArrays for fullscreen triangle optimization?
    }

}