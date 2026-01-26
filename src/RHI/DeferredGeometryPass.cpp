#include "DeferredGeometryPass.h"
#include "../Core/ShaderCompiler.h"
#include "../Core/Log.h"
#include "StandardPBRMaterial.h"
#include "OpenGL/OpenGLResources.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>

namespace AEngine {

    static std::shared_ptr<IRHIPipelineState> CreateGeometryPSO() {
        auto readFile = [](const std::string& path) {
            std::ifstream file(path);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        };

        std::string vertSrc = readFile("shaders/DeferredGeometry.vert");
        std::string fragSrc = readFile("shaders/DeferredGeometry.frag");

        // Use direct GLSL compilation for now (since we disabled SPIR-V in StandardPBRMaterial for stability)
        auto compileShader = [](GLenum type, const std::string& src) -> GLuint {
            GLuint shader = glCreateShader(type);
            const char* srcPtr = src.c_str();
            glShaderSource(shader, 1, &srcPtr, nullptr);
            glCompileShader(shader);
            
            GLint success;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                char infoLog[512];
                glGetShaderInfoLog(shader, 512, nullptr, infoLog);
                AE_CORE_ERROR("Deferred Shader Compilation Failed: {0}", infoLog);
                return 0;
            }
            return shader;
        };

        GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertSrc);
        GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragSrc);

        if (!vertShader || !fragShader) return nullptr;

        GLuint program = glCreateProgram();
        glAttachShader(program, vertShader);
        glAttachShader(program, fragShader);
        glLinkProgram(program);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);

        return std::make_shared<FOpenGLPipelineState>(program);
    }

    FDeferredGeometryPass::FDeferredGeometryPass(std::shared_ptr<IRHIFramebuffer> gBuffer)
        : m_gBuffer(gBuffer) {
        m_pipelineState = CreateGeometryPSO();
    }

    FDeferredGeometryPass::~FDeferredGeometryPass() {}

    /// @brief Renders opaque geometry into the G-Buffer.
    /// This pass outputs to 3 Color Attachments (MRT):
    /// - Location 0: Albedo (RGB) + Specular (A)
    /// - Location 1: Normal (World Space, RGB)
    /// - Location 2: Emissive (RGB) + Metallic/Roughness (Packed?) - currently only Emissive logic is explicit.
    void FDeferredGeometryPass::Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) {
        if (!m_gBuffer || !m_pipelineState) return;

        m_gBuffer->Bind();
        cmdBuffer.SetViewport(0, 0, m_gBuffer->GetDepthAttachment()->GetWidth(), m_gBuffer->GetDepthAttachment()->GetHeight());
        cmdBuffer.Clear(0.0f, 0.0f, 0.0f, 1.0f); // Clear G-Buffer

        cmdBuffer.SetPipelineState(m_pipelineState);
        
        for (const auto& renderable : renderables) {
            if (!renderable.Material) continue;

            // We need to update the material parameters (matrices) first
            // Note: StandardPBRMaterial stores these internally. 
            // We use SetParameter to update them, then BindUniforms uploads them.
            renderable.Material->SetParameter("model_matrix", renderable.WorldMatrix);
            renderable.Material->SetParameter("view_matrix", context.ViewMatrix);
            renderable.Material->SetParameter("projection_matrix", context.ProjectionMatrix);

            // Bind Uniforms to CURRENT program (DeferredGeometry)
            // This works because we aligned locations (20+) in both shaders.
            if (auto pbrMat = std::dynamic_pointer_cast<FStandardPBRMaterial>(renderable.Material)) {
                pbrMat->BindUniforms();
            }

            cmdBuffer.SetVertexBuffer(renderable.VertexBuffer);
            cmdBuffer.SetIndexBuffer(renderable.IndexBuffer);
            cmdBuffer.DrawIndexed(renderable.IndexCount);
        }

        m_gBuffer->Unbind();
    }

}
