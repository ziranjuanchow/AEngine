#include "DeferredLightingPass.h"
#include "Engine.RHI/ShaderCompiler.h"
#include "Kernel/Core/Log.h"
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

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

    }

    FDeferredLightingPass::~FDeferredLightingPass() {}

    void FDeferredLightingPass::Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) {
        if (!m_pipelineState || !m_gBuffer || !m_sphereVB || !m_sphereIB || m_sphereIndexCount == 0) {
            return;
        }

        if (m_outputFramebuffer) {
            m_outputFramebuffer->Bind();
            cmdBuffer.SetViewport(0, 0, m_width, m_height);
            cmdBuffer.Clear(0.0f, 0.0f, 0.0f, 1.0f, false);
        }

        // Additive blending accumulates one light volume draw call per light.
        cmdBuffer.SetBlendState(true);
        cmdBuffer.SetBlendFunc(ERHIBlendFactor::One, ERHIBlendFactor::One);
        cmdBuffer.SetDepthTest(true, false, ERHICompareFunc::LessEqual);
        cmdBuffer.SetCullMode(ERHICullMode::Front);

        auto albedoRough = m_gBuffer->GetColorAttachment(0);
        auto normalMetal = m_gBuffer->GetColorAttachment(1);
        auto depth = m_gBuffer->GetDepthAttachment();
        if (!albedoRough || !normalMetal || !depth) {
            return;
        }
        albedoRough->Bind(0);
        normalMetal->Bind(1);
        depth->Bind(2);

        cmdBuffer.SetPipelineState(m_pipelineState);
        cmdBuffer.SetVertexBuffer(m_sphereVB);
        cmdBuffer.SetIndexBuffer(m_sphereIB);

        const glm::mat4 invVP = glm::inverse(context.ProjectionMatrix * context.ViewMatrix);
        cmdBuffer.SetUniform(20, 0); // gAlbedoRough
        cmdBuffer.SetUniform(21, 1); // gNormalMetal
        cmdBuffer.SetUniform(22, 2); // gDepth
        cmdBuffer.SetUniform(23, invVP);
        cmdBuffer.SetUniform(24, context.CameraPosition);
        cmdBuffer.SetUniform(25, glm::vec2(static_cast<float>(m_width), static_cast<float>(m_height)));
        cmdBuffer.SetUniform(30, context.LightPosition);
        cmdBuffer.SetUniform(31, context.LightColor);

        const auto emitLight = [&](const glm::vec3& position, const glm::vec3& color, float radius, float intensity) {
            const glm::mat4 model = glm::translate(glm::mat4(1.0f), position) *
                                    glm::scale(glm::mat4(1.0f), glm::vec3(radius));
            cmdBuffer.SetUniform(0, model);
            cmdBuffer.SetUniform(1, context.ViewMatrix);
            cmdBuffer.SetUniform(2, context.ProjectionMatrix);
            cmdBuffer.SetUniform(26, position);
            cmdBuffer.SetUniform(27, color);
            cmdBuffer.SetUniform(28, radius);
            cmdBuffer.SetUniform(29, intensity);
            cmdBuffer.DrawIndexed(m_sphereIndexCount);
        };

        if (!context.PointLights.empty()) {
            const size_t maxLights = std::min<size_t>(32, context.PointLights.size());
            for (size_t i = 0; i < maxLights; ++i) {
                const auto& light = context.PointLights[i];
                emitLight(light.Position, light.Color, light.Radius, light.Intensity);
            }
        } else {
            emitLight(context.LightPosition, context.LightColor, 10.0f, 1.0f);
        }

        if (m_outputFramebuffer) {
            m_outputFramebuffer->Unbind();
        }
    }

}
