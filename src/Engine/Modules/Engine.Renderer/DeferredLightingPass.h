#pragma once

#include "RenderGraph.h"
#include "RHIResources.h"

namespace AEngine {

    class FDeferredLightingPass : public FRenderPass {
    public:
        FDeferredLightingPass(std::shared_ptr<IRHIDevice> device, std::shared_ptr<IRHIFramebuffer> gBuffer, std::shared_ptr<IRHITexture> shadowMap);
        virtual ~FDeferredLightingPass();

        virtual void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) override;
        virtual std::string GetName() const override { return "DeferredLightingPass"; }

        void SetResolution(uint32_t width, uint32_t height) { m_width = width; m_height = height; }
        void SetOutputTarget(std::shared_ptr<IRHIFramebuffer> outputFramebuffer) { m_outputFramebuffer = outputFramebuffer; }
        
        void SetSphereGeometry(std::shared_ptr<IRHIBuffer> vb, std::shared_ptr<IRHIBuffer> ib, uint32_t count) {
            m_sphereVB = vb;
            m_sphereIB = ib;
            m_sphereIndexCount = count;
        }

        uint32_t GetLastCandidateLightCount() const { return m_lastCandidateLightCount; }
        uint32_t GetLastVisibleLightCount() const { return m_lastVisibleLightCount; }

    private:
        std::shared_ptr<IRHIFramebuffer> m_gBuffer;
        std::shared_ptr<IRHIFramebuffer> m_outputFramebuffer;
        std::shared_ptr<IRHITexture> m_shadowMap;
        std::shared_ptr<IRHIPipelineState> m_pipelineState;
        
        std::shared_ptr<IRHIBuffer> m_sphereVB;
        std::shared_ptr<IRHIBuffer> m_sphereIB;
        uint32_t m_sphereIndexCount = 0;
        uint32_t m_width = 1;
        uint32_t m_height = 1;
        uint32_t m_lastCandidateLightCount = 0;
        uint32_t m_lastVisibleLightCount = 0;
    };

}
