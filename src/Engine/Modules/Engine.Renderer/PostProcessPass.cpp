#include "PostProcessPass.h"
#include "Kernel/Core/Log.h"
#include "Engine.RHI/ShaderCompiler.h"
#include <fstream>
#include <sstream>

namespace AEngine {

    FPostProcessPass::FPostProcessPass(std::shared_ptr<IRHIDevice> device)
        : m_device(device) {
        auto readFile = [](const std::string& path) {
            std::ifstream file(path);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        };

        std::string vertSrc = readFile("shaders/PostProcess.vert");
        std::string fragSrc = readFile("shaders/ACES_ToneMapping.frag");

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
            AE_CORE_ERROR("Failed to compile PostProcess shaders.");
        }
    }

    FPostProcessPass::~FPostProcessPass() {}

    void FPostProcessPass::Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) {
        if (m_inputTexture) {
            Execute(cmdBuffer, m_inputTexture);
        }
    }

    void FPostProcessPass::Execute(IRHICommandBuffer& cmdBuffer, std::shared_ptr<IRHITexture> inputTexture) {
        if (!m_pipelineState || !inputTexture || !m_device) return;

        if (m_outputFramebuffer) {
            m_outputFramebuffer->Bind();
        } else {
            m_device->BindDefaultFramebuffer();
        }
        cmdBuffer.SetViewport(0, 0, m_width, m_height);
        cmdBuffer.Clear(0.0f, 0.0f, 0.0f, 1.0f, true);
        cmdBuffer.SetDepthTest(false, false, ERHICompareFunc::LessEqual);
        cmdBuffer.SetCullMode(ERHICullMode::None);
        cmdBuffer.SetBlendState(false);

        inputTexture->Bind(0);

        cmdBuffer.SetPipelineState(m_pipelineState);
        cmdBuffer.SetUniform(0, 0); 
        cmdBuffer.SetUniform(1, 1.0f); // Default Exposure
        cmdBuffer.Draw(3);

        if (m_outputFramebuffer) {
            m_outputFramebuffer->Unbind();
        }
    }

}
