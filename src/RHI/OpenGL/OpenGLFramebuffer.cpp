#include "OpenGLFramebuffer.h"
#include "OpenGLResources.h"
#include "../../Core/Log.h"

namespace AEngine {

    FOpenGLFramebuffer::FOpenGLFramebuffer(const FFramebufferConfig& config)
        : m_width(config.Width), m_height(config.Height), m_depthAttachment(config.DepthAttachment), m_colorAttachments(config.ColorAttachments) {
        
        glCreateFramebuffers(1, &m_handle);

        std::vector<GLenum> drawBuffers;

        for (size_t i = 0; i < m_colorAttachments.size(); ++i) {
            auto* glTex = static_cast<FOpenGLTexture*>(m_colorAttachments[i].get());
            glNamedFramebufferTexture(m_handle, GL_COLOR_ATTACHMENT0 + (GLenum)i, glTex->GetHandle(), 0);
            drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + (GLenum)i);
        }

        if (!drawBuffers.empty()) {
            glNamedFramebufferDrawBuffers(m_handle, (GLsizei)drawBuffers.size(), drawBuffers.data());
        } else {
            glNamedFramebufferDrawBuffer(m_handle, GL_NONE);
            glNamedFramebufferReadBuffer(m_handle, GL_NONE);
        }

        if (m_depthAttachment) {
            auto* glDepth = static_cast<FOpenGLTexture*>(m_depthAttachment.get());
            glNamedFramebufferTexture(m_handle, GL_DEPTH_ATTACHMENT, glDepth->GetHandle(), 0);
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
