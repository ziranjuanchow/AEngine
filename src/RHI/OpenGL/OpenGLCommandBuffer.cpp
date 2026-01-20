#include "OpenGLCommandBuffer.h"
#include "OpenGLResources.h"

namespace AEngine {

    void FOpenGLCommandBuffer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        glViewport(x, y, width, height);
    }

    void FOpenGLCommandBuffer::Clear(float r, float g, float b, float a) {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void FOpenGLCommandBuffer::SetPipelineState(std::shared_ptr<IRHIPipelineState> pso) {
        if (pso) {
            auto* glPSO = static_cast<FOpenGLPipelineState*>(pso.get());
            glUseProgram(glPSO->GetProgram());
        }
    }

    void FOpenGLCommandBuffer::SetVertexBuffer(std::shared_ptr<IRHIBuffer> buffer) {
        if (buffer) {
            auto* glBuffer = static_cast<FOpenGLBuffer*>(buffer.get());
            glBindBuffer(GL_ARRAY_BUFFER, glBuffer->GetHandle());
        }
    }

    void FOpenGLCommandBuffer::SetIndexBuffer(std::shared_ptr<IRHIBuffer> buffer) {
        if (buffer) {
            auto* glBuffer = static_cast<FOpenGLBuffer*>(buffer.get());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->GetHandle());
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
