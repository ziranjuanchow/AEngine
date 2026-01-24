#pragma once

#include "../IRHIDevice.h" // For FFramebufferConfig
#include <glad/glad.h>
#include <vector>

namespace AEngine {

    class FOpenGLFramebuffer : public IRHIFramebuffer {
    public:
        FOpenGLFramebuffer(const FFramebufferConfig& config);
        virtual ~FOpenGLFramebuffer();

        virtual void Bind() override;
        virtual void Unbind() override;
        virtual std::shared_ptr<IRHITexture> GetDepthAttachment() const override { return m_depthAttachment; }
        virtual std::shared_ptr<IRHITexture> GetColorAttachment(uint32_t index) const override { 
            if (index < m_colorAttachments.size()) return m_colorAttachments[index];
            return nullptr;
        }

        GLuint GetHandle() const { return m_handle; }

    private:
        GLuint m_handle = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        std::shared_ptr<IRHITexture> m_depthAttachment;
        std::vector<std::shared_ptr<IRHITexture>> m_colorAttachments;
    };

}
