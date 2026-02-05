#include "DeferredGeometryPass.h"
#include "Engine.RHI/ShaderCompiler.h"
#include "Kernel/Core/Log.h"
#include "StandardPBRMaterial.h"
#include <fstream>
#include <sstream>

namespace AEngine {

    FDeferredGeometryPass::FDeferredGeometryPass(std::shared_ptr<IRHIDevice> device, std::shared_ptr<IRHIFramebuffer> gBuffer)
        : m_gBuffer(gBuffer) {
        
        auto readFile = [](const std::string& path) {
            std::ifstream file(path);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        };

        std::string vertSrc = readFile("shaders/DeferredGeometry.vert");
        std::string fragSrc = readFile("shaders/DeferredGeometry.frag");

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
            AE_CORE_ERROR("Failed to compile DeferredGeometry shaders.");
        }
    }

    FDeferredGeometryPass::~FDeferredGeometryPass() {}

    void FDeferredGeometryPass::Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) {
        if (!m_gBuffer || !m_pipelineState) return;

        m_gBuffer->Bind();
        // Set draw buffers for G-Buffer MRT
        cmdBuffer.SetDrawBuffers({ERHIPixelFormat::RGBA8_UNORM, ERHIPixelFormat::RGBA16_FLOAT, ERHIPixelFormat::RGBA8_UNORM});
        cmdBuffer.SetViewport(0, 0, m_gBuffer->GetDepthAttachment()->GetWidth(), m_gBuffer->GetDepthAttachment()->GetHeight());
        cmdBuffer.Clear(0.0f, 0.0f, 0.0f, 1.0f); 

        cmdBuffer.SetPipelineState(m_pipelineState);
        
        for (const auto& renderable : renderables) {
            if (!renderable.Material) continue;

            renderable.Material->SetParameter("model_matrix", renderable.WorldMatrix);
            renderable.Material->SetParameter("view_matrix", context.ViewMatrix);
            renderable.Material->SetParameter("projection_matrix", context.ProjectionMatrix);

            if (auto pbrMat = std::dynamic_pointer_cast<FStandardPBRMaterial>(renderable.Material)) {
                // Warning: This still calls glUniform directly inside. 
                // But it doesn't new FOpenGL* so it should link fine.
                pbrMat->BindUniforms();
            }

            cmdBuffer.SetVertexBuffer(renderable.VertexBuffer);
            cmdBuffer.SetIndexBuffer(renderable.IndexBuffer);
            cmdBuffer.DrawIndexed(renderable.IndexCount);
        }

        m_gBuffer->Unbind();
    }

}