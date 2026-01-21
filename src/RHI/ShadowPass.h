#pragma once

#include "RenderGraph.h"
#include "RHIResources.h"

namespace AEngine {

    class FShadowPass : public FRenderPass {
    public:
        FShadowPass(std::shared_ptr<IRHIFramebuffer> framebuffer);
        virtual ~FShadowPass();

        virtual void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) override;
        virtual std::string GetName() const override { return "ShadowPass"; }

        glm::mat4 GetLightSpaceMatrix() const { return m_lightSpaceMatrix; }
        std::shared_ptr<IRHITexture> GetDepthMap() const { return m_framebuffer->GetDepthAttachment(); }

    private:
        std::shared_ptr<IRHIFramebuffer> m_framebuffer;
        std::shared_ptr<IRHIPipelineState> m_pipelineState;
        glm::mat4 m_lightSpaceMatrix;
    };

}
