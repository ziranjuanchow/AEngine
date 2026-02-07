#pragma once

#include "RenderGraph.h"
#include "RHIResources.h"

namespace AEngine {

    class FPostProcessPass : public FRenderPass {
    public:
        FPostProcessPass(std::shared_ptr<IRHIDevice> device);
        ~FPostProcessPass();

        virtual void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) override;
        void Execute(IRHICommandBuffer& cmdBuffer, std::shared_ptr<IRHITexture> inputTexture);
        virtual std::string GetName() const override { return "PostProcessPass"; }

        void SetExposure(float exposure) { m_exposure = exposure; }
        void SetInputTexture(std::shared_ptr<IRHITexture> input) { m_inputTexture = input; }
        void SetOutputTarget(std::shared_ptr<IRHIFramebuffer> outputFramebuffer) { m_outputFramebuffer = outputFramebuffer; }
        void SetResolution(uint32_t width, uint32_t height) { m_width = width; m_height = height; }

    private:
        std::shared_ptr<IRHIDevice> m_device;
        std::shared_ptr<IRHITexture> m_inputTexture;
        std::shared_ptr<IRHIFramebuffer> m_outputFramebuffer;
        std::shared_ptr<IRHIPipelineState> m_pipelineState;
        float m_exposure = 1.0f;
        uint32_t m_width = 1;
        uint32_t m_height = 1;
    };

}
