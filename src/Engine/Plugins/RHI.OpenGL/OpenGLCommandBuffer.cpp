#include "OpenGLCommandBuffer.h"
#include "OpenGLResources.h"
#include "Kernel/Core/Log.h"
#include <glm/gtc/type_ptr.hpp>

namespace AEngine {

    // Helper to convert ERHIBlendFactor to GLenum
    static GLenum ConvertBlendFactor(ERHIBlendFactor factor) {
        switch (factor) {
            case ERHIBlendFactor::Zero: return GL_ZERO;
            case ERHIBlendFactor::One: return GL_ONE;
            case ERHIBlendFactor::SrcColor: return GL_SRC_COLOR;
            case ERHIBlendFactor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
            case ERHIBlendFactor::DstColor: return GL_DST_COLOR;
            case ERHIBlendFactor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
            case ERHIBlendFactor::SrcAlpha: return GL_SRC_ALPHA;
            case ERHIBlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
            case ERHIBlendFactor::DstAlpha: return GL_DST_ALPHA;
            case ERHIBlendFactor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
            case ERHIBlendFactor::ConstantColor: return GL_CONSTANT_COLOR;
            case ERHIBlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
            case ERHIBlendFactor::ConstantAlpha: return GL_CONSTANT_ALPHA;
            case ERHIBlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
            case ERHIBlendFactor::SrcAlphaSaturate: return GL_SRC_ALPHA_SATURATE;
            default:
                AE_CORE_ERROR("Unknown ERHIBlendFactor!");
                return GL_ONE; // Fallback
        }
    }

    static GLenum ConvertCompareFunc(ERHICompareFunc func) {
        switch (func) {
            case ERHICompareFunc::Never: return GL_NEVER;
            case ERHICompareFunc::Less: return GL_LESS;
            case ERHICompareFunc::Equal: return GL_EQUAL;
            case ERHICompareFunc::LessEqual: return GL_LEQUAL;
            case ERHICompareFunc::Greater: return GL_GREATER;
            case ERHICompareFunc::NotEqual: return GL_NOTEQUAL;
            case ERHICompareFunc::GreaterEqual: return GL_GEQUAL;
            case ERHICompareFunc::Always: return GL_ALWAYS;
            default:
                AE_CORE_ERROR("Unknown ERHICompareFunc!");
                return GL_LEQUAL;
        }
    }

    static GLenum ConvertCullMode(ERHICullMode mode) {
        switch (mode) {
            case ERHICullMode::Front: return GL_FRONT;
            case ERHICullMode::Back: return GL_BACK;
            case ERHICullMode::FrontAndBack: return GL_FRONT_AND_BACK;
            case ERHICullMode::None:
            default:
                return GL_NONE;
        }
    }

    void FOpenGLCommandBuffer::Begin() {
        // No-op for immediate GL
    }

    void FOpenGLCommandBuffer::End() {
        // No-op
    }

    void FOpenGLCommandBuffer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        glViewport(x, y, width, height);
    }

    void FOpenGLCommandBuffer::Clear(float r, float g, float b, float a, bool clearDepth) {
        glClearColor(r, g, b, a);
        GLbitfield mask = GL_COLOR_BUFFER_BIT;
        if (clearDepth) {
            glDepthMask(GL_TRUE); // Ensure we can clear
            mask |= GL_DEPTH_BUFFER_BIT;
        }
        glClear(mask);
    }

    void FOpenGLCommandBuffer::SetDrawBuffers(const std::vector<ERHIPixelFormat>& formats) {
        if (formats.empty()) {
            glDrawBuffer(GL_NONE); // Disable color writes
            glReadBuffer(GL_NONE);
            return;
        }

        std::vector<GLenum> drawBuffers(formats.size());
        for (size_t i = 0; i < formats.size(); ++i) {
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
        }
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    }

    void FOpenGLCommandBuffer::SetDepthBias(float constant, float slope) {
        if (constant != 0.0f || slope != 0.0f) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(slope, constant); // Note param order
        } else {
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
    }

    void FOpenGLCommandBuffer::SetBlendState(bool enabled) {
        if (enabled) {
            glEnable(GL_BLEND);
            // Default blend func if not set explicitly via SetBlendFunc
            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // This line is now redundant
        } else {
            glDisable(GL_BLEND);
        }
    }

    void FOpenGLCommandBuffer::SetBlendFunc(ERHIBlendFactor sfactor, ERHIBlendFactor dfactor) {
        glBlendFunc(ConvertBlendFactor(sfactor), ConvertBlendFactor(dfactor));
    }

    void FOpenGLCommandBuffer::SetDepthTest(bool enabled, bool writeEnabled, ERHICompareFunc func) {
        if (enabled) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
        
        glDepthMask(writeEnabled ? GL_TRUE : GL_FALSE);
        glDepthFunc(ConvertCompareFunc(func));
    }

    void FOpenGLCommandBuffer::SetCullMode(ERHICullMode mode) {
        if (mode == ERHICullMode::None) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
            glCullFace(ConvertCullMode(mode));
        }
    }

    void FOpenGLCommandBuffer::SetPipelineState(std::shared_ptr<IRHIPipelineState> pso) {
        if (auto glPSO = std::dynamic_pointer_cast<FOpenGLPipelineState>(pso)) {
            if (m_currentProgram != glPSO->GetProgram()) {
                glPSO->Bind();
                m_currentProgram = glPSO->GetProgram();
            }
        }
    }

    void FOpenGLCommandBuffer::SetVertexBuffer(std::shared_ptr<IRHIBuffer> buffer) {
        // Quick & Dirty VAO setup for immediate mode
        if (auto glBuf = std::dynamic_pointer_cast<FOpenGLBuffer>(buffer)) {
            // In a real engine, VAO should be baked in InputLayout/Pipeline
            if (m_currentVAO == 0) {
                glCreateVertexArrays(1, &m_currentVAO);
                glBindVertexArray(m_currentVAO);
            }
            
            if (m_currentVBO != glBuf->GetHandle()) {
                glBindBuffer(GL_ARRAY_BUFFER, glBuf->GetHandle());

                // Standard FVertex layout: P(3), N(3), UV(2), T(3), B(3), C(4)
                const int stride = sizeof(float) * 18;
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)*3));
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)*6));
                glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)*8));
                glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)*11));
                glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)*14));
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glEnableVertexAttribArray(2);
                glEnableVertexAttribArray(3);
                glEnableVertexAttribArray(4);
                glEnableVertexAttribArray(5);

                m_currentVBO = glBuf->GetHandle();
            }
        }
    }

    void FOpenGLCommandBuffer::SetIndexBuffer(std::shared_ptr<IRHIBuffer> buffer) {
        if (auto glBuf = std::dynamic_pointer_cast<FOpenGLBuffer>(buffer)) {
            if (m_currentIndexBuffer != glBuf->GetHandle()) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuf->GetHandle());
                m_currentIndexBuffer = glBuf->GetHandle();
            }
        }
    }

    void FOpenGLCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount) {
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    void FOpenGLCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount) {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }

    void FOpenGLCommandBuffer::SetUniform(uint32_t location, const glm::mat4& value) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }

    void FOpenGLCommandBuffer::SetUniform(uint32_t location, const glm::vec2& value) {
        glUniform2fv(location, 1, glm::value_ptr(value));
    }

    void FOpenGLCommandBuffer::SetUniform(uint32_t location, const glm::vec3& value) {
        glUniform3fv(location, 1, glm::value_ptr(value));
    }

    void FOpenGLCommandBuffer::SetUniform(uint32_t location, int value) {
        glUniform1i(location, value);
    }

    void FOpenGLCommandBuffer::SetUniform(uint32_t location, float value) {
        glUniform1f(location, value);
    }

}
