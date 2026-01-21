#pragma once

#include "../IRHIDevice.h"
#include <glad/glad.h>

namespace AEngine {

    class FOpenGLCommandBuffer : public IRHICommandBuffer {
    public:
        virtual void Begin() override {}
        virtual void End() override {}

        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
        virtual void Clear(float r, float g, float b, float a) override;
        
        virtual void SetPipelineState(std::shared_ptr<IRHIPipelineState> pso) override;
        virtual void SetVertexBuffer(std::shared_ptr<IRHIBuffer> buffer) override;
        virtual void SetIndexBuffer(std::shared_ptr<IRHIBuffer> buffer) override;

        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1) override;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) override;

    private:
        GLuint m_currentVAO = 0; 
        GLuint m_currentProgram = 0;
        GLuint m_currentVBO = 0;
        GLuint m_currentIndexBuffer = 0;
    };

}
