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

    FOpenGLTexture::FOpenGLTexture(uint32_t width, uint32_t height, ERHIPixelFormat format, const void* data)
        : m_width(width), m_height(height) {
        
        glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
        
        GLenum internalFormat = GL_RGBA8;
        GLenum externalFormat = GL_RGBA;
        GLenum type = GL_UNSIGNED_BYTE;

        switch (format) {
            case ERHIPixelFormat::RGBA16_FLOAT: 
                internalFormat = GL_RGBA16F; type = GL_HALF_FLOAT; break;
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
        
        // Setup shadow parameters for Depth textures
        if (format == ERHIPixelFormat::Depth24) {
            glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTextureParameterfv(m_handle, GL_TEXTURE_BORDER_COLOR, borderColor);
            
            // Enable hardware PCF comparison
            glTextureParameteri(m_handle, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTextureParameteri(m_handle, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        }

        if (data) {
            glTextureSubImage2D(m_handle, 0, 0, 0, width, height, externalFormat, type, data);
        }
    }

    FOpenGLTexture::~FOpenGLTexture() {
        glDeleteTextures(1, &m_handle);
    }

}
