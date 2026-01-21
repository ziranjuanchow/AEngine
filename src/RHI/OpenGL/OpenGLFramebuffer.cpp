#include "OpenGLFramebuffer.h"
#include "OpenGLResources.h"
#include "../../Core/Log.h"

namespace AEngine {

    FOpenGLFramebuffer::FOpenGLFramebuffer(uint32_t width, uint32_t height, std::shared_ptr<IRHITexture> depthAttachment)
        : m_width(width), m_height(height), m_depthAttachment(depthAttachment) {
        
        glCreateFramebuffers(1, &m_handle);

        if (m_depthAttachment) {
            auto* glDepth = static_cast<FOpenGLTexture*>(m_depthAttachment.get());
            glNamedFramebufferTexture(m_handle, GL_DEPTH_ATTACHMENT, glDepth->GetHandle(), 0);
            
            // Explicitly tell OpenGL we are not rendering color
            glNamedFramebufferDrawBuffer(m_handle, GL_NONE);
            glNamedFramebufferReadBuffer(m_handle, GL_NONE);
        }

        GLenum status = glCheckNamedFramebufferStatus(m_handle, GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            AE_CORE_ERROR("Framebuffer is incomplete! Status: {0}", status);
        }
    }

    FOpenGLFramebuffer::~FOpenGLFramebuffer() {
        glDeleteFramebuffers(1, &m_handle);
    }

    void FOpenGLFramebuffer::Bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_handle);
        glViewport(0, 0, m_width, m_height);
    }

    void FOpenGLFramebuffer::Unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

}
