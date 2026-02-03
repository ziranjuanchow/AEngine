#include "OpenGLResources.h"
#include <cstring>

namespace AEngine {

    FOpenGLBuffer::FOpenGLBuffer(ERHIBufferType type, uint32_t size, ERHIBufferUsage usage, const void* data)
        : m_size(size) {
        
        switch (type) {
            case ERHIBufferType::Vertex: m_glType = GL_ARRAY_BUFFER; break;
            case ERHIBufferType::Index:  m_glType = GL_ELEMENT_ARRAY_BUFFER; break;
            case ERHIBufferType::Uniform: m_glType = GL_UNIFORM_BUFFER; break;
        }

        glCreateBuffers(1, &m_handle);
        glNamedBufferStorage(m_handle, size, data, (usage == ERHIBufferUsage::Dynamic) ? GL_DYNAMIC_STORAGE_BIT : 0);
    }

    FOpenGLBuffer::~FOpenGLBuffer() {
        glDeleteBuffers(1, &m_handle);
    }

    void FOpenGLBuffer::Bind() {
        glBindBuffer(m_glType, m_handle);
    }

    void FOpenGLBuffer::Unbind() {
        glBindBuffer(m_glType, 0);
    }

    FOpenGLTexture::FOpenGLTexture(uint32_t width, uint32_t height, ERHIPixelFormat format, const void* data)
        : m_width(width), m_height(height), m_format(format) {
        
        glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
        
        GLenum internalFormat = GL_RGBA8;
        GLenum externalFormat = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;

        switch (format) {
            case ERHIPixelFormat::RGBA16_FLOAT: 
                internalFormat = GL_RGBA16F; type = GL_HALF_FLOAT; break;
            case ERHIPixelFormat::RGBA8_UNORM:
                internalFormat = GL_RGBA8; type = GL_UNSIGNED_BYTE; break;
            case ERHIPixelFormat::D24_S8: 
                internalFormat = GL_DEPTH24_STENCIL8; 
                externalFormat = GL_DEPTH_STENCIL;
                type = GL_UNSIGNED_INT_24_8;
                break;
            case ERHIPixelFormat::Depth24:
                internalFormat = GL_DEPTH_COMPONENT24;
                externalFormat = GL_DEPTH_COMPONENT;
                type = GL_FLOAT;
                break;
        }

        glTextureStorage2D(m_handle, 1, internalFormat, width, height);
        
        if (format == ERHIPixelFormat::Depth24) {
            glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTextureParameterfv(m_handle, GL_TEXTURE_BORDER_COLOR, borderColor);
            glTextureParameteri(m_handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTextureParameteri(m_handle, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        } else {
            glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        if (data) {
            glTextureSubImage2D(m_handle, 0, 0, 0, width, height, externalFormat, type, data);
        }
    }

    FOpenGLTexture::~FOpenGLTexture() {
        glDeleteTextures(1, &m_handle);
    }

    void FOpenGLTexture::Bind(uint32_t slot) {
        glBindTextureUnit(slot, m_handle);
    }

    FOpenGLPipelineState::FOpenGLPipelineState(GLuint program)
        : m_program(program) {
    }

    FOpenGLPipelineState::~FOpenGLPipelineState() {
        glDeleteProgram(m_program);
    }

    void FOpenGLPipelineState::Bind() {
        glUseProgram(m_program);
    }

    void FOpenGLPipelineState::Unbind() {
        glUseProgram(0);
    }

}