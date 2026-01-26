#pragma once

#include <memory>
#include <vector>
#include "RHIDefinitions.h"

namespace AEngine {

    /// @brief Base interface for all GPU resources.
    class IRHIResource {
    public:
        virtual ~IRHIResource() = default;
    };

    /// @brief Abstraction for GPU Buffers (Vertex, Index, Uniform).
    class IRHIBuffer : public IRHIResource {
    public:
        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual uint32_t GetSize() const = 0;
    };

    /// @brief Abstraction for 2D Textures and Render Targets.
    class IRHITexture : public IRHIResource {
    public:
        /// @brief Binds texture to a specific slot (GL_TEXTURE0 + slot).
        virtual void Bind(uint32_t slot) = 0;
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual ERHIPixelFormat GetFormat() const = 0;
    };

    /// @brief Abstraction for Framebuffer Objects (FBO).
    /// Manages a collection of Texture Attachments for MRT (Multiple Render Targets).
    class IRHIFramebuffer : public IRHIResource {
    public:
        /// @brief Binds this FBO for writing.
        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual std::shared_ptr<IRHITexture> GetDepthAttachment() const = 0;
        /// @brief Gets a color attachment by index (e.g., 0 for Albedo in G-Buffer).
        virtual std::shared_ptr<IRHITexture> GetColorAttachment(uint32_t index) const = 0;
    };

    /// @brief Abstraction for Pipeline State Objects (PSO).
    /// Currently wraps the Shader Program.
    class IRHIPipelineState : public IRHIResource {
    public:
        virtual void Bind() = 0;
        virtual void Unbind() = 0;
    };

}
