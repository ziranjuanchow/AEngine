#pragma once

#include <memory>
#include <vector>
#include <span>
#include "RHIResources.h"

namespace AEngine {

    /**
     * @brief Abstraction for recording rendering commands.
     * 
     * Encapsulates draw calls, state changes, and resource binding.
     */
    class IRHICommandBuffer {
    public:
        virtual ~IRHICommandBuffer() = default;

        virtual void Begin() = 0;
        virtual void End() = 0;

        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
        virtual void Clear(float r, float g, float b, float a) = 0;
        virtual void SetDepthBias(float constant, float slope) = 0;
        
        virtual void SetPipelineState(std::shared_ptr<IRHIPipelineState> pso) = 0;
        virtual void SetVertexBuffer(std::shared_ptr<IRHIBuffer> buffer) = 0;
        virtual void SetIndexBuffer(std::shared_ptr<IRHIBuffer> buffer) = 0;

        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) = 0;
    };

    struct FFramebufferConfig {
        uint32_t Width;
        uint32_t Height;
        std::shared_ptr<IRHITexture> DepthAttachment;
        // ColorAttachments can be added later
    };

    /**
     * @brief Abstraction for the Graphics Device.
     * 
     * Responsible for creating resources (buffers, textures) and managing command buffers.
     */
    class IRHIDevice {
    public:
        virtual ~IRHIDevice() = default;

        virtual std::shared_ptr<IRHIBuffer> CreateBuffer(ERHIBufferType type, uint32_t size, ERHIBufferUsage usage, const void* data = nullptr) = 0;
        virtual std::shared_ptr<IRHITexture> CreateTexture(uint32_t width, uint32_t height, ERHIPixelFormat format, const void* data = nullptr) = 0;
        virtual std::shared_ptr<IRHIFramebuffer> CreateFramebuffer(const FFramebufferConfig& config) = 0;
        
        virtual std::shared_ptr<IRHICommandBuffer> CreateCommandBuffer() = 0;
        
        virtual void SubmitCommandBuffer(std::shared_ptr<IRHICommandBuffer> cmdBuffer) = 0;
        virtual void Present() = 0;
    };

}
