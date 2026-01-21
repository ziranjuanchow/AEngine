#include "ShadowPass.h"
#include "../Core/ShaderCompiler.h"
#include "../Core/Log.h"
#include "OpenGL/OpenGLResources.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>

namespace AEngine {

    static std::shared_ptr<IRHIPipelineState> CreateShadowPSO() {
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
            return nullptr;
        }

        GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderBinary(1, &vertShader, GL_SHADER_BINARY_FORMAT_SPIR_V, vertSpv->data(), (GLsizei)(vertSpv->size() * sizeof(uint32_t)));
        glSpecializeShader(vertShader, "main", 0, nullptr, nullptr);

        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderBinary(1, &fragShader, GL_SHADER_BINARY_FORMAT_SPIR_V, fragSpv->data(), (GLsizei)(fragSpv->size() * sizeof(uint32_t)));
        glSpecializeShader(fragShader, "main", 0, nullptr, nullptr);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        return std::make_shared<FOpenGLPipelineState>(program);
    }

    FShadowPass::FShadowPass(std::shared_ptr<IRHIFramebuffer> framebuffer)
        : m_framebuffer(framebuffer) {
        m_pipelineState = CreateShadowPSO();
    }

    FShadowPass::~FShadowPass() {}

    void FShadowPass::Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) {
        if (!m_framebuffer || !m_pipelineState) return;

        m_framebuffer->Bind();
        cmdBuffer.SetViewport(0, 0, m_framebuffer->GetDepthAttachment()->GetWidth(), m_framebuffer->GetDepthAttachment()->GetHeight());
        glClear(GL_DEPTH_BUFFER_BIT); // Direct GL clear for depth only framebuffer

        cmdBuffer.SetPipelineState(m_pipelineState);
        
        // Upload LightSpaceMatrix (Location 0)
        auto* glPSO = static_cast<FOpenGLPipelineState*>(m_pipelineState.get());
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(context.LightSpaceMatrix));

        cmdBuffer.SetDepthBias(1.1f, 4.0f); // Default bias values

        for (const auto& renderable : renderables) {
            // Upload Model Matrix (Location 1)
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(renderable.WorldMatrix));

            cmdBuffer.SetVertexBuffer(renderable.VertexBuffer);
            cmdBuffer.SetIndexBuffer(renderable.IndexBuffer);
            cmdBuffer.DrawIndexed(renderable.IndexCount);
        }

        cmdBuffer.SetDepthBias(0.0f, 0.0f);
        m_framebuffer->Unbind();
    }

}
