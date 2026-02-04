#include "PostProcessPass.h"
#include "Kernel/Core/Log.h"
#include "Engine.RHI/ShaderCompiler.h"
#include <fstream>
#include <sstream>

namespace AEngine {

    FPostProcessPass::FPostProcessPass(std::shared_ptr<IRHIDevice> device) {
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
        if (!m_pipelineState || !inputTexture) return;

        inputTexture->Bind(0);

        cmdBuffer.SetPipelineState(m_pipelineState);
        cmdBuffer.SetUniform(0, 0); 
        cmdBuffer.Draw(3);
    }

}
