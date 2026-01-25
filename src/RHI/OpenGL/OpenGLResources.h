#pragma once

#include "../RHIResources.h"
#include <glad/glad.h>

namespace AEngine {

    class FOpenGLBuffer : public IRHIBuffer {
    public:
        // Use RHI types in constructor to match .cpp implementation
        FOpenGLBuffer(ERHIBufferType type, uint32_t size, ERHIBufferUsage usage, const void* data);
        virtual ~FOpenGLBuffer();

        virtual void Bind() override;
        virtual void Unbind() override;
        virtual uint32_t GetSize() const override { return m_size; }

        GLuint GetHandle() const { return m_handle; }

    private:
        GLuint m_handle = 0;
        GLenum m_target;
        GLenum m_glType; // Added member
        uint32_t m_size; // Added member
    };

    class FOpenGLTexture : public IRHITexture {
    public:
        FOpenGLTexture(uint32_t width, uint32_t height, ERHIPixelFormat format, const void* data = nullptr);
        virtual ~FOpenGLTexture();

        virtual void Bind(uint32_t slot) override;
        virtual uint32_t GetWidth() const override { return m_width; }
        virtual uint32_t GetHeight() const override { return m_height; }
        virtual ERHIPixelFormat GetFormat() const override { return m_format; }

        GLuint GetHandle() const { return m_handle; }

    private:
        GLuint m_handle = 0;
        uint32_t m_width;
        uint32_t m_height;
        ERHIPixelFormat m_format;
    };

    class FOpenGLPipelineState : public IRHIPipelineState {
    public:
        FOpenGLPipelineState(GLuint program);
        virtual ~FOpenGLPipelineState();

        virtual void Bind() override;
        virtual void Unbind() override;

        GLuint GetProgram() const { return m_program; }

    private:
        GLuint m_program = 0;
    };

}
