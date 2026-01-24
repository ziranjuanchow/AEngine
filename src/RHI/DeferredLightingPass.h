#pragma once

#include "RenderGraph.h"
#include "RHIResources.h"

namespace AEngine {

    class FDeferredLightingPass : public FRenderPass {
    public:
        FDeferredLightingPass(std::shared_ptr<IRHIFramebuffer> gBuffer, std::shared_ptr<IRHIBuffer> sphereVB, std::shared_ptr<IRHIBuffer> sphereIB, uint32_t sphereIndexCount);
        virtual ~FDeferredLightingPass();

        virtual void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) override;
        virtual std::string GetName() const override { return "DeferredLightingPass"; }

    private:
        std::shared_ptr<IRHIFramebuffer> m_gBuffer;
        std::shared_ptr<IRHIPipelineState> m_pipelineState;
        
        std::shared_ptr<IRHIBuffer> m_sphereVB;
        std::shared_ptr<IRHIBuffer> m_sphereIB;
        uint32_t m_sphereIndexCount;
    };

}
