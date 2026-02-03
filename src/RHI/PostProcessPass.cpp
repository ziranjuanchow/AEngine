#include "PostProcessPass.h"
#include "Kernel/Core/Log.h"
#include "OpenGL/OpenGLResources.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>

namespace AEngine {

    static std::shared_ptr<IRHIPipelineState> CreatePostProcessPSO() {
        auto readFile = [](const std::string& path) {
            std::ifstream file(path);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        };

        std::string vertSrc = readFile("shaders/PostProcess.vert");
        std::string fragSrc = readFile("shaders/ACES_ToneMapping.frag");

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
                AE_CORE_ERROR("PostProcess Shader Compilation Failed: {0}", infoLog);
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

    FPostProcessPass::FPostProcessPass(std::shared_ptr<IRHITexture> inputTexture)
        : m_inputTexture(inputTexture) {
        m_pipelineState = CreatePostProcessPSO();
    }

    FPostProcessPass::~FPostProcessPass() {}

    void FPostProcessPass::Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) {
        if (!m_inputTexture || !m_pipelineState) return;

        // Render to Screen (Default FBO assumed bound)
        // Or we should assume RenderGraph handles binding.
        // For PostProcess, we usually render to screen at the end.
        
        cmdBuffer.SetBlendState(false);
        cmdBuffer.SetDepthTest(false, false);
        cmdBuffer.SetCullMode(0);

        cmdBuffer.SetPipelineState(m_pipelineState);
        auto* glPSO = static_cast<FOpenGLPipelineState*>(m_pipelineState.get());
        GLuint program = glPSO->GetProgram();

        // Bind Input Texture
        auto* glTex = static_cast<FOpenGLTexture*>(m_inputTexture.get());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, glTex->GetHandle());
        glUniform1i(0, 0); // sceneTexture

        glUniform1f(1, m_exposure);

        // Fullscreen Triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

}
