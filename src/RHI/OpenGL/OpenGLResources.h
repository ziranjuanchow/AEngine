#pragma once

#include <glad/glad.h>
#include "../RHIResources.h"

namespace AEngine {

    class FOpenGLBuffer : public IRHIBuffer {
    public:
        FOpenGLBuffer(ERHIBufferType type, uint32_t size, ERHIBufferUsage usage, const void* data);
        virtual ~FOpenGLBuffer();

        virtual uint32_t GetSize() const override { return m_size; }
        GLuint GetHandle() const { return m_handle; }
        GLenum GetGLType() const { return m_glType; }

    private:
        GLuint m_handle = 0;
        uint32_t m_size = 0;
        GLenum m_glType = 0;
    };

    class FOpenGLTexture : public IRHITexture {
    public:
        FOpenGLTexture(uint32_t width, uint32_t height, ERHIPixelFormat format, const void* data);
        virtual ~FOpenGLTexture();

        virtual uint32_t GetWidth() const override { return m_width; }
        virtual uint32_t GetHeight() const override { return m_height; }
        GLuint GetHandle() const { return m_handle; }

    private:
        GLuint m_handle = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
    };

    class FOpenGLPipelineState : public IRHIPipelineState {
    public:
        // OpenGL doesn't have a direct PSO, but we can store shader programs here
        FOpenGLPipelineState(GLuint program) : m_program(program) {}
        virtual ~FOpenGLPipelineState() { glDeleteProgram(m_program); }
        GLuint GetProgram() const { return m_program; }

    private:
        GLuint m_program = 0;
    };

}
