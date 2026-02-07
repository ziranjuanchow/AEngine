#pragma once

#include "RenderGraph.h"
#include "RHIResources.h"

namespace AEngine {

    class FForwardLitPass : public FRenderPass {
    public:
        void SetOutputTarget(std::shared_ptr<IRHIFramebuffer> outputFramebuffer) { m_outputFramebuffer = outputFramebuffer; }
        void SetResolution(uint32_t width, uint32_t height) { m_width = width; m_height = height; }

        virtual void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) override;
        virtual std::string GetName() const override { return "ForwardLitPass"; }

    private:
        std::shared_ptr<IRHIFramebuffer> m_outputFramebuffer;
        uint32_t m_width = 1;
        uint32_t m_height = 1;
    };

}
