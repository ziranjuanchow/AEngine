#pragma once

#include "RenderGraph.h"
#include "RHIResources.h"
#include "IRHIDevice.h"
#include <memory>
#include <vector>

namespace AEngine {

    class FRenderPass;
    class FShadowPass;
    class FDeferredGeometryPass;
    class FDeferredLightingPass;
    class FForwardLitPass;
    class FPostProcessPass;

    class FSceneRenderer {
    public:
        FSceneRenderer(std::shared_ptr<IRHIDevice> device);
        ~FSceneRenderer();

        void Init(uint32_t width, uint32_t height);
        void Shutdown();
        void Resize(uint32_t width, uint32_t height);

        void Render(const FRenderContext& context, 
                    const std::vector<FRenderable>& deferredList,
                    const std::vector<FRenderable>& forwardList);

        // Getters for textures (e.g. for debugging UI)
        std::shared_ptr<IRHITexture> GetGBufferAlbedo() const;
        std::shared_ptr<IRHITexture> GetGBufferNormal() const;
        std::shared_ptr<IRHITexture> GetGBufferDepth() const;
        std::shared_ptr<IRHITexture> GetHDRColor() const;
        uint32_t GetDeferredLightingCandidateLights() const;
        uint32_t GetDeferredLightingVisibleLights() const;

    private:
        std::shared_ptr<IRHIDevice> m_device;
        std::unique_ptr<FRenderGraph> m_renderGraph;
        std::shared_ptr<IRHICommandBuffer> m_cmdBuffer;

        // FBOs
        std::shared_ptr<IRHIFramebuffer> m_gBuffer;
        std::shared_ptr<IRHIFramebuffer> m_hdrLightingFBO;
        std::shared_ptr<IRHIFramebuffer> m_hdrForwardFBO;
        
        // Pass references (owned by RenderGraph)
        FShadowPass* m_shadowPass = nullptr;
        FDeferredGeometryPass* m_geometryPass = nullptr;
        FDeferredLightingPass* m_lightingPass = nullptr;
        FForwardLitPass* m_forwardPass = nullptr;
        FPostProcessPass* m_postProcessPass = nullptr;

        uint32_t m_width = 0;
        uint32_t m_height = 0;

        void CreateFBOs(uint32_t width, uint32_t height);
    };

}
