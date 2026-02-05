#include "OpenGLDevice.h"
#include "OpenGLResources.h"
#include "OpenGLCommandBuffer.h"
#include "OpenGLFramebuffer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Helper to convert ERHIBlitMask to GLbitfield
static GLbitfield ConvertBlitMask(AEngine::ERHIBlitMask mask) {
    GLbitfield glMask = 0;
    if (static_cast<uint32_t>(mask) & static_cast<uint32_t>(AEngine::ERHIBlitMask::ColorBuffer)) glMask |= GL_COLOR_BUFFER_BIT;
    if (static_cast<uint32_t>(mask) & static_cast<uint32_t>(AEngine::ERHIBlitMask::DepthBuffer)) glMask |= GL_DEPTH_BUFFER_BIT;
    if (static_cast<uint32_t>(mask) & static_cast<uint32_t>(AEngine::ERHIBlitMask::StencilBuffer)) glMask |= GL_STENCIL_BUFFER_BIT;
    return glMask;
}

// Helper to convert ERHIBlitFilter to GLenum
static GLenum ConvertBlitFilter(AEngine::ERHIBlitFilter filter) {
    switch (filter) {
        case AEngine::ERHIBlitFilter::Nearest: return GL_NEAREST;
        case AEngine::ERHIBlitFilter::Linear: return GL_LINEAR;
        default: return GL_NEAREST; // Fallback
    }
}

namespace AEngine { // Reopen namespace for FOpenGLDevice methods

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

    void FOpenGLDevice::BlitFramebuffer(std::shared_ptr<IRHIFramebuffer> source, std::shared_ptr<IRHIFramebuffer> destination, 
                                     uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight, 
                                     ERHIBlitMask mask, ERHIBlitFilter filter) {
        GLuint srcFBO = source ? static_cast<FOpenGLFramebuffer*>(source.get())->GetHandle() : 0;
        GLuint dstFBO = destination ? static_cast<FOpenGLFramebuffer*>(destination.get())->GetHandle() : 0;

        glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFBO);
        glBlitFramebuffer(0, 0, srcWidth, srcHeight, 0, 0, dstWidth, dstHeight, ConvertBlitMask(mask), ConvertBlitFilter(filter));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // Unbind FBOs
    }

    void FOpenGLDevice::Present() {
        // SwapBuffers is handled by WindowModule currently.
        // In full RHI, Device should own the SwapChain.
    }

} // namespace AEngine