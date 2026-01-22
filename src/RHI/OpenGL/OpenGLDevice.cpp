#include "OpenGLDevice.h"
#include "OpenGLResources.h"
#include "OpenGLCommandBuffer.h"
#include "OpenGLFramebuffer.h"
#include <GLFW/glfw3.h>

namespace AEngine {

    std::shared_ptr<IRHIBuffer> FOpenGLDevice::CreateBuffer(ERHIBufferType type, uint32_t size, ERHIBufferUsage usage, const void* data) {
        return std::make_shared<FOpenGLBuffer>(type, size, usage, data);
    }

    std::shared_ptr<IRHITexture> FOpenGLDevice::CreateTexture(uint32_t width, uint32_t height, ERHIPixelFormat format, const void* data) {
        return std::make_shared<FOpenGLTexture>(width, height, format, data);
    }

    std::shared_ptr<IRHIFramebuffer> FOpenGLDevice::CreateFramebuffer(const FFramebufferConfig& config) {
        return std::make_shared<FOpenGLFramebuffer>(config);
    }

    std::shared_ptr<IRHICommandBuffer> FOpenGLDevice::CreateCommandBuffer() {
        return std::make_shared<FOpenGLCommandBuffer>();
    }

    void FOpenGLDevice::Present() {
        // Swap buffers is handled by WindowSubsystem currently, but RHI could take it
    }

}
