#pragma once
#include "Engine.RHI/IRHIDevice.h"
#include <memory>

namespace AEngine {

    class FOpenGLDevice : public IRHIDevice {
    public:
        FOpenGLDevice();
        virtual ~FOpenGLDevice();

        virtual std::shared_ptr<IRHIBuffer> CreateBuffer(ERHIBufferType type, uint32_t size, ERHIBufferUsage usage, const void* data = nullptr) override;
        virtual std::shared_ptr<IRHITexture> CreateTexture(uint32_t width, uint32_t height, ERHIPixelFormat format, const void* data = nullptr) override;
        virtual std::shared_ptr<IRHIFramebuffer> CreateFramebuffer(const FFramebufferConfig& config) override;
        
        virtual std::shared_ptr<IRHIShader> CreateShader(const std::vector<uint32_t>& spirv, ERHIShaderStage stage) override;
        virtual std::shared_ptr<IRHIPipelineState> CreatePipelineState(const FPipelineStateDesc& desc) override;

        virtual std::shared_ptr<IRHICommandBuffer> CreateCommandBuffer() override;
        
        virtual void SubmitCommandBuffer(std::shared_ptr<IRHICommandBuffer> cmdBuffer) override;
        virtual void BlitFramebuffer(std::shared_ptr<IRHIFramebuffer> source, uint32_t width, uint32_t height) override;
        virtual void Present() override;
    };

}