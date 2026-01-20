#pragma once

#include "../IRHIDevice.h"

namespace AEngine {

    class FOpenGLDevice : public IRHIDevice {
    public:
        virtual std::shared_ptr<IRHIBuffer> CreateBuffer(ERHIBufferType type, uint32_t size, ERHIBufferUsage usage, const void* data = nullptr) override;
        virtual std::shared_ptr<IRHITexture> CreateTexture(uint32_t width, uint32_t height, ERHIPixelFormat format, const void* data = nullptr) override;
        
        virtual std::shared_ptr<IRHICommandBuffer> CreateCommandBuffer() override;
        
        virtual void SubmitCommandBuffer(std::shared_ptr<IRHICommandBuffer> cmdBuffer) override {}
        virtual void Present() override;
    };

}
