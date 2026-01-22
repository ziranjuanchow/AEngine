#pragma once

#include "RenderGraph.h"
#include "RHIResources.h"

namespace AEngine {

    class FDeferredGeometryPass : public FRenderPass {
    public:
        FDeferredGeometryPass(std::shared_ptr<IRHIFramebuffer> gBuffer);
        virtual ~FDeferredGeometryPass();

        virtual void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) override;
        virtual std::string GetName() const override { return "DeferredGeometryPass"; }

    private:
        std::shared_ptr<IRHIFramebuffer> m_gBuffer;
        std::shared_ptr<IRHIPipelineState> m_pipelineState;
    };

}
