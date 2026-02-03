#pragma once

#include "RenderGraph.h"
#include "RHIResources.h"

namespace AEngine {

    class FPostProcessPass : public FRenderPass {
    public:
        FPostProcessPass(std::shared_ptr<IRHITexture> inputTexture);
        virtual ~FPostProcessPass();

        virtual void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) override;
        virtual std::string GetName() const override { return "PostProcessPass"; }

        void SetExposure(float exposure) { m_exposure = exposure; }
        void SetInputTexture(std::shared_ptr<IRHITexture> input) { m_inputTexture = input; }

    private:
        std::shared_ptr<IRHITexture> m_inputTexture;
        std::shared_ptr<IRHIPipelineState> m_pipelineState;
        float m_exposure = 1.0f;
    };

}
