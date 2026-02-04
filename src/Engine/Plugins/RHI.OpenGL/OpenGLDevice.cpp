#include "OpenGLDevice.h"
#include "OpenGLResources.h"
#include "OpenGLCommandBuffer.h"
#include "OpenGLFramebuffer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace AEngine {

    FOpenGLDevice::FOpenGLDevice() {
        // Assume context is initialized by WindowModule
    }

    FOpenGLDevice::~FOpenGLDevice() {
    }

    std::shared_ptr<IRHIBuffer> FOpenGLDevice::CreateBuffer(ERHIBufferType type, uint32_t size, ERHIBufferUsage usage, const void* data) {
        return std::make_shared<FOpenGLBuffer>(type, size, usage, data);
    }

    std::shared_ptr<IRHITexture> FOpenGLDevice::CreateTexture(uint32_t width, uint32_t height, ERHIPixelFormat format, const void* data) {
        return std::make_shared<FOpenGLTexture>(width, height, format, data);
    }

    std::shared_ptr<IRHIFramebuffer> FOpenGLDevice::CreateFramebuffer(const FFramebufferConfig& config) {
        return std::make_shared<FOpenGLFramebuffer>(config);
    }

    std::shared_ptr<IRHIShader> FOpenGLDevice::CreateShader(const std::vector<uint32_t>& spirv, ERHIShaderStage stage) {
        return std::make_shared<FOpenGLShader>(spirv, stage);
    }

    std::shared_ptr<IRHIPipelineState> FOpenGLDevice::CreatePipelineState(const FPipelineStateDesc& desc) {
        return std::make_shared<FOpenGLPipelineState>(desc);
    }

    std::shared_ptr<IRHICommandBuffer> FOpenGLDevice::CreateCommandBuffer() {
        return std::make_shared<FOpenGLCommandBuffer>();
    }

    void FOpenGLDevice::SubmitCommandBuffer(std::shared_ptr<IRHICommandBuffer> cmdBuffer) {
        // In OpenGL, commands are executed immediately when called.
        // But for structure, we simulate submission.
        if (auto glCmd = std::dynamic_pointer_cast<FOpenGLCommandBuffer>(cmdBuffer)) {
            // If we had a deferred command list, we would execute it here.
            // For now, FOpenGLCommandBuffer executes immediately.
        }
    }

    void FOpenGLDevice::BlitFramebuffer(std::shared_ptr<IRHIFramebuffer> source, uint32_t width, uint32_t height) {
        if (auto glFBO = std::dynamic_pointer_cast<FOpenGLFramebuffer>(source)) {
            glBlitNamedFramebuffer(glFBO->GetHandle(), 0, 
                0, 0, width, height, 
                0, 0, width, height, 
                GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        }
    }

    void FOpenGLDevice::Present() {
        // SwapBuffers is handled by WindowModule currently.
        // In full RHI, Device should own the SwapChain.
    }

}