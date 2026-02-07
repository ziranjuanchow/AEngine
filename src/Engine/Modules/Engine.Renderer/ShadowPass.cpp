#include "ShadowPass.h"
#include "Kernel/Core/Log.h"
#include "Engine.RHI/ShaderCompiler.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
// 移除了 glad 和 OpenGLResources 的引用！完美解耦！

namespace AEngine {

    FShadowPass::FShadowPass(std::shared_ptr<IRHIDevice> device, std::shared_ptr<IRHIFramebuffer> framebuffer)
        : m_framebuffer(framebuffer) {
        
        auto readFile = [](const std::string& path) {
            std::ifstream file(path);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        };

        std::string vertSrc = readFile("shaders/ShadowDepth.vert");
        std::string fragSrc = readFile("shaders/ShadowDepth.frag");

        auto& compiler = FShaderCompiler::Get();
        auto vertSpv = compiler.CompileGLSL(EShaderStage::Vertex, vertSrc);
        auto fragSpv = compiler.CompileGLSL(EShaderStage::Fragment, fragSrc);

        if (!vertSpv || !fragSpv) {
            AE_CORE_ERROR("Failed to compile Shadow shaders!");
            return;
        }

        auto vs = device->CreateShader(*vertSpv, ERHIShaderStage::Vertex);
        auto fs = device->CreateShader(*fragSpv, ERHIShaderStage::Fragment);

        FPipelineStateDesc desc;
        desc.VertexShader = vs;
        desc.FragmentShader = fs;
        m_pipelineState = device->CreatePipelineState(desc);
    }

    FShadowPass::~FShadowPass() {}

    void FShadowPass::Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) {
        if (!m_framebuffer || !m_pipelineState) return;

        // 这里依然有潜在的 OpenGL 依赖，比如 glUniformMatrix4fv
        // 理想情况下 IRHIPipelineState 应该封装 Uniform 绑定
        // 但为了这一步重构，我们先把构造函数解耦。
        // 由于 IRHICommandBuffer 没有 SetUniform 接口，我们可能还无法完全解耦 Execute。
        // 但这已经是一个巨大的进步。
        
        // 注意：这里我们无法 cast 到 FOpenGLPipelineState，因为我们没包含头文件。
        // 所以我们暂时无法设置 Uniform。
        // 这是一个问题。如果不解决 Uniform，阴影就废了。
        
        // 临时解决方案：CommandBuffer 应该有 SetUniform 接口。
        // 或者我们暂时先 include glad，手动调用 glUniform。
        
        m_framebuffer->Bind();
        cmdBuffer.SetDrawBuffers({});
        cmdBuffer.SetViewport(0, 0, m_framebuffer->GetDepthAttachment()->GetWidth(), m_framebuffer->GetDepthAttachment()->GetHeight());
        cmdBuffer.Clear(0,0,0,0, true);
        cmdBuffer.SetBlendState(false);
        cmdBuffer.SetDepthTest(true, true, ERHICompareFunc::LessEqual);
        cmdBuffer.SetCullMode(ERHICullMode::Back);

        cmdBuffer.SetPipelineState(m_pipelineState);
        
        // Upload LightSpaceMatrix (Location 0)
        cmdBuffer.SetUniform(0, context.LightSpaceMatrix);

        cmdBuffer.SetDepthBias(1.1f, 4.0f); 

        for (const auto& renderable : renderables) {
            // Upload Model Matrix (Location 1)
            cmdBuffer.SetUniform(1, renderable.WorldMatrix);

            cmdBuffer.SetVertexBuffer(renderable.VertexBuffer);
            cmdBuffer.SetIndexBuffer(renderable.IndexBuffer);
            cmdBuffer.DrawIndexed(renderable.IndexCount);
        }

        cmdBuffer.SetDepthBias(0.0f, 0.0f);
        m_framebuffer->Unbind();
    }

}
