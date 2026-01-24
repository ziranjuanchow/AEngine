#include "OpenGLCommandBuffer.h"
#include "OpenGLResources.h"
#include <cstddef>

namespace AEngine {

    void FOpenGLCommandBuffer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        glViewport(x, y, width, height);
    }

    void FOpenGLCommandBuffer::Clear(float r, float g, float b, float a) {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void FOpenGLCommandBuffer::SetDepthBias(float constant, float slope) {
        if (constant != 0.0f || slope != 0.0f) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(constant, slope);
        } else {
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
    }

    void FOpenGLCommandBuffer::SetBlendState(bool enabled) {
        if (enabled) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE); // Additive
        } else {
            glDisable(GL_BLEND);
        }
    }

    void FOpenGLCommandBuffer::SetDepthTest(bool enabled, bool writeEnabled, uint32_t func) {
        if (enabled) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        glDepthMask(writeEnabled ? GL_TRUE : GL_FALSE);
        glDepthFunc(func);
    }

    void FOpenGLCommandBuffer::SetCullMode(uint32_t mode) {
        if (mode == 0) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
            glCullFace(mode);
        }
    }

    void FOpenGLCommandBuffer::SetPipelineState(std::shared_ptr<IRHIPipelineState> pso) {
        if (pso) {
            auto* glPSO = static_cast<FOpenGLPipelineState*>(pso.get());
            GLuint program = glPSO->GetProgram();
            
            if (m_currentProgram != program) {
                glUseProgram(program);
                m_currentProgram = program;
            }
        }
    }

    void FOpenGLCommandBuffer::SetVertexBuffer(std::shared_ptr<IRHIBuffer> buffer) {
        if (buffer) {
            auto* glBuffer = static_cast<FOpenGLBuffer*>(buffer.get());
            GLuint vbo = glBuffer->GetHandle();

            if (m_currentVAO == 0) {
                glGenVertexArrays(1, &m_currentVAO);
            }
            glBindVertexArray(m_currentVAO);
            
            if (m_currentVBO != vbo) {
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                m_currentVBO = vbo;

                // Position
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FVertex), (void*)offsetof(FVertex, Position));
                // Normal
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FVertex), (void*)offsetof(FVertex, Normal));
                // TexCoords
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(FVertex), (void*)offsetof(FVertex, TexCoords));
                // Tangent
                glEnableVertexAttribArray(3);
                glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FVertex), (void*)offsetof(FVertex, Tangent));
                // Bitangent
                glEnableVertexAttribArray(4);
                glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(FVertex), (void*)offsetof(FVertex, Bitangent));
                // Color
                glEnableVertexAttribArray(5);
                glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(FVertex), (void*)offsetof(FVertex, Color));
            }
        }
    }

    void FOpenGLCommandBuffer::SetIndexBuffer(std::shared_ptr<IRHIBuffer> buffer) {
        if (buffer) {
            auto* glBuffer = static_cast<FOpenGLBuffer*>(buffer.get());
            GLuint ibo = glBuffer->GetHandle();

            if (m_currentIndexBuffer != ibo) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
                m_currentIndexBuffer = ibo;
            }
        }
    }

    void FOpenGLCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount) {
        if (instanceCount > 1)
            glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, instanceCount);
        else
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    void FOpenGLCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount) {
        if (instanceCount > 1)
            glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
        else
            glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    }

}
