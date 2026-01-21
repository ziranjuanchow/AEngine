#pragma once

#include "../RHIResources.h"
#include <glad/glad.h>

namespace AEngine {

    class FOpenGLFramebuffer : public IRHIFramebuffer {
    public:
        FOpenGLFramebuffer(uint32_t width, uint32_t height, std::shared_ptr<IRHITexture> depthAttachment);
        virtual ~FOpenGLFramebuffer();

        virtual void Bind() override;
        virtual void Unbind() override;
        virtual std::shared_ptr<IRHITexture> GetDepthAttachment() const override { return m_depthAttachment; }

    private:
        GLuint m_handle = 0;
        uint32_t m_width = 0;
        uint32_t m_height = 0;
        std::shared_ptr<IRHITexture> m_depthAttachment;
    };

}
