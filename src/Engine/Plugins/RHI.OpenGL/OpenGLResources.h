#pragma once
#include "Engine.RHI/RHIResources.h"
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
        GLenum m_glType; 
        uint32_t m_size; 
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

    class FOpenGLShader : public IRHIShader {
    public:
        FOpenGLShader(const std::vector<uint32_t>& spirv, ERHIShaderStage stage);
        virtual ~FOpenGLShader();
        virtual ERHIShaderStage GetStage() const override { return m_stage; }
        GLuint GetHandle() const { return m_handle; }
    private:
        GLuint m_handle = 0;
        ERHIShaderStage m_stage;
    };

    class FOpenGLPipelineState : public IRHIPipelineState {
    public:
        // Legacy constructor for raw program ID (deprecated but keep for compilation if needed)
        // FOpenGLPipelineState(GLuint program);
        
        // New abstract constructor
        FOpenGLPipelineState(const FPipelineStateDesc& desc);
        virtual ~FOpenGLPipelineState();

        virtual void Bind() override;
        virtual void Unbind() override;

        GLuint GetProgram() const { return m_program; }

    private:
        GLuint m_program = 0;
    };

}