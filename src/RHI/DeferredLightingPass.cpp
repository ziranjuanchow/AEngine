#include "DeferredLightingPass.h"
#include "../Core/ShaderCompiler.h"
#include "Kernel/Core/Log.h"
#include "OpenGL/OpenGLResources.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>

namespace AEngine {

    static std::shared_ptr<IRHIPipelineState> CreateLightingPSO() {
        auto readFile = [](const std::string& path) {
            std::ifstream file(path);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        };

        std::string vertSrc = readFile("shaders/DeferredLighting.vert");
        std::string fragSrc = readFile("shaders/DeferredLighting.frag");

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
                AE_CORE_ERROR("Lighting Shader Compilation Failed: {0}", infoLog);
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

    FDeferredLightingPass::FDeferredLightingPass(std::shared_ptr<IRHIFramebuffer> gBuffer, std::shared_ptr<IRHIBuffer> sphereVB, std::shared_ptr<IRHIBuffer> sphereIB, uint32_t sphereIndexCount, uint32_t width, uint32_t height)
        : m_gBuffer(gBuffer), m_sphereVB(sphereVB), m_sphereIB(sphereIB), m_sphereIndexCount(sphereIndexCount), m_width(width), m_height(height) {
        m_pipelineState = CreateLightingPSO();
    }

    FDeferredLightingPass::~FDeferredLightingPass() {}

    /// @brief Performs deferred lighting calculation using light volumes.
    /// Technique:
    /// 1. Render sphere geometry representing the light volume.
    /// 2. Bind G-Buffer textures (Albedo, Normal, Depth) as inputs.
    /// 3. Calculate PBR lighting per-fragment inside the sphere volume.
    /// 4. Use Additive Blending (ONE, ONE) to accumulate light contributions.
    void FDeferredLightingPass::Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) {
        if (!m_gBuffer || !m_pipelineState) return;

        // Ensure Viewport matches G-Buffer size
        cmdBuffer.SetViewport(0, 0, m_width, m_height);

        // Note: We render to the screen (default FBO assumed to be bound before pass if not using specific RT)
        
        cmdBuffer.SetBlendState(true);
        // Disable Depth Test (We have no depth buffer bound to avoid feedback loop)
        // We rely on Shader distance check for culling logic.
        cmdBuffer.SetDepthTest(false, false, GL_ALWAYS);
        
        // Render BACK faces so we can see the volume when inside it
        // TODO: Refactor to RHI (This direct GL enum usage leaks abstraction)
        cmdBuffer.SetCullMode(GL_FRONT); 

        cmdBuffer.SetPipelineState(m_pipelineState);
        auto* glPSO = static_cast<FOpenGLPipelineState*>(m_pipelineState.get());
        GLuint program = glPSO->GetProgram();

        // 1. Bind G-Buffer Textures
        // TODO: Refactor binding logic to RHI ResourceBindings
        auto bindGBuffer = [&](int loc, int unit, std::shared_ptr<IRHITexture> tex) {
            auto* glTex = static_cast<FOpenGLTexture*>(tex.get());
            glActiveTexture(GL_TEXTURE0 + unit);
            glBindTexture(GL_TEXTURE_2D, glTex->GetHandle());
            glUniform1i(loc, unit);
        };

        bindGBuffer(20, 0, m_gBuffer->GetColorAttachment(0)); // AlbedoRough
        bindGBuffer(21, 1, m_gBuffer->GetColorAttachment(1)); // NormalMetal
        bindGBuffer(22, 2, m_gBuffer->GetDepthAttachment());  // Depth

        // 2. Global Uniforms
        // TODO: Move to Uniform Buffer Object (UBO)
        glm::mat4 invVP = glm::inverse(context.ProjectionMatrix * context.ViewMatrix);
        glUniformMatrix4fv(23, 1, GL_FALSE, glm::value_ptr(invVP));
        glUniform3fv(24, 1, glm::value_ptr(context.CameraPosition));
        
        // Viewport size for UV reconstruction
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glUniform2f(25, (float)viewport[2], (float)viewport[3]);

        glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(context.ViewMatrix));
        glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(context.ProjectionMatrix));

        cmdBuffer.SetVertexBuffer(m_sphereVB);
        cmdBuffer.SetIndexBuffer(m_sphereIB);

        // 3. Render each light volume
        for (const auto& light : context.PointLights) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), light.Position) * 
                              glm::scale(glm::mat4(1.0f), glm::vec3(light.Radius));
            
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(model));
            glUniform3fv(26, 1, glm::value_ptr(light.Position));
            glUniform3fv(27, 1, glm::value_ptr(light.Color));
            glUniform1f(28, light.Radius);
            glUniform1f(29, light.Intensity);

            cmdBuffer.DrawIndexed(m_sphereIndexCount);
        }

        // Cleanup
        // Unbind G-Buffer textures to avoid feedback loop in subsequent passes (e.g. Forward Pass using Depth)
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);

        cmdBuffer.SetBlendState(false);
        cmdBuffer.SetDepthTest(true, true, GL_LEQUAL);
        cmdBuffer.SetCullMode(GL_BACK);
    }

}
