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
     * Designed to be backend-agnostic (OpenGL, Vulkan, DX12).
     */
    class IRHICommandBuffer {
    public:
        virtual ~IRHICommandBuffer() = default;

        /// @brief Starts command recording.
        virtual void Begin() = 0;
        /// @brief Ends command recording.
        virtual void End() = 0;

        // --- Render State Commands ---
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
        virtual void Clear(float r, float g, float b, float a, bool clearDepth = true) = 0;
        /// @brief Specifies a list of color buffers to be drawn into.
        virtual void SetDrawBuffers(const std::vector<ERHIPixelFormat>& formats) = 0;
        
        /// @brief Sets Depth Bias (Polygon Offset). Used for Shadow Mapping to prevent acne.
        virtual void SetDepthBias(float constant, float slope) = 0;
        
        /// @brief Enables/Disables Blending.
        virtual void SetBlendState(bool enabled) = 0;
        /// @brief Configures the blending function for combining source and destination pixels.
        virtual void SetBlendFunc(ERHIBlendFactor sfactor, ERHIBlendFactor dfactor) = 0;
        
        /// @brief Configures Depth Testing and Writing.
        /// @param func Comparison function (default GL_LEQUAL = 0x0203).
        virtual void SetDepthTest(bool enabled, bool writeEnabled, uint32_t func = 0x0203) = 0; 
        
        /// @brief Sets Face Culling Mode.
        /// @param mode GL_BACK (0x0405) or GL_FRONT (0x0404).
        virtual void SetCullMode(uint32_t mode) = 0; 
        
        // --- Binding Commands ---
        virtual void SetPipelineState(std::shared_ptr<IRHIPipelineState> pso) = 0;
        virtual void SetVertexBuffer(std::shared_ptr<IRHIBuffer> buffer) = 0;
        virtual void SetIndexBuffer(std::shared_ptr<IRHIBuffer> buffer) = 0;
        
        virtual void SetUniform(uint32_t location, const glm::mat4& value) = 0;
        virtual void SetUniform(uint32_t location, const glm::vec3& value) = 0;
        virtual void SetUniform(uint32_t location, int value) = 0;
        virtual void SetUniform(uint32_t location, float value) = 0;

        // --- Draw Commands ---
        virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1) = 0;
        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) = 0;
    };

    /// @brief Configuration struct for Framebuffer creation.
    struct FFramebufferConfig {
        uint32_t Width;
        uint32_t Height;
        std::shared_ptr<IRHITexture> DepthAttachment;
        std::vector<std::shared_ptr<IRHITexture>> ColorAttachments;
    };

    /**
     * @brief Abstraction for the Graphics Device.
     * 
     * Responsible for creating resources (buffers, textures) and managing command buffers.
     */
    class IRHIDevice {
    public:
        virtual ~IRHIDevice() = default;

        // --- Resource Creation ---
        virtual std::shared_ptr<IRHIBuffer> CreateBuffer(ERHIBufferType type, uint32_t size, ERHIBufferUsage usage, const void* data = nullptr) = 0;
        virtual std::shared_ptr<IRHITexture> CreateTexture(uint32_t width, uint32_t height, ERHIPixelFormat format, const void* data = nullptr) = 0;
        virtual std::shared_ptr<IRHIFramebuffer> CreateFramebuffer(const FFramebufferConfig& config) = 0;
        
        virtual std::shared_ptr<IRHIShader> CreateShader(const std::vector<uint32_t>& spirv, ERHIShaderStage stage) = 0;
        virtual std::shared_ptr<IRHIPipelineState> CreatePipelineState(const FPipelineStateDesc& desc) = 0;

        virtual std::shared_ptr<IRHICommandBuffer> CreateCommandBuffer() = 0;
        
        // --- Execution ---
        virtual void SubmitCommandBuffer(std::shared_ptr<IRHICommandBuffer> cmdBuffer) = 0;
        
        /// @brief Blits (Copies) one framebuffer to another.
        /// Commonly used to copy Depth Buffer from G-Buffer to Forward Pass FBO.
        virtual void BlitFramebuffer(std::shared_ptr<IRHIFramebuffer> source, uint32_t width, uint32_t height) = 0;
        
        /// @brief Swaps the back buffer (VSync).
        virtual void Present() = 0;
    };

}
