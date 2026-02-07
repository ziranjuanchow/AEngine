#include "SceneRenderer.h"
#include "ShadowPass.h"
#include "DeferredGeometryPass.h"
#include "DeferredLightingPass.h"
#include "ForwardLitPass.h"
#include "PostProcessPass.h"
#include "StandardPBRMaterial.h"
#include "Kernel/Core/Log.h"
#include "Engine.Scene/GeometryUtils.h"

namespace AEngine {

    FSceneRenderer::FSceneRenderer(std::shared_ptr<IRHIDevice> device)
        : m_device(device) {
        m_renderGraph = std::make_unique<FRenderGraph>();
        m_cmdBuffer = m_device->CreateCommandBuffer();
    }

    FSceneRenderer::~FSceneRenderer() {
        Shutdown();
    }

    void FSceneRenderer::Init(uint32_t width, uint32_t height) {
        m_width = width;
        m_height = height;

        CreateFBOs(width, height);

        // 1. Shadow Pass
        FFramebufferConfig shadowConfig;
        shadowConfig.Width = 2048;
        shadowConfig.Height = 2048;
        shadowConfig.DepthAttachment = m_device->CreateTexture(2048, 2048, ERHIPixelFormat::Depth24);
        auto shadowFBO = m_device->CreateFramebuffer(shadowConfig);
        auto shadowPassPtr = std::make_unique<FShadowPass>(m_device, shadowFBO);
        m_shadowPass = shadowPassPtr.get();
        m_renderGraph->AddPass(std::move(shadowPassPtr));

        // 2. Deferred Geometry Pass
        auto geomPassPtr = std::make_unique<FDeferredGeometryPass>(m_device, m_gBuffer);
        m_geometryPass = geomPassPtr.get();
        m_renderGraph->AddPass(std::move(geomPassPtr));

        // 3. Deferred Lighting Pass
        std::shared_ptr<IRHIBuffer> sphereVB, sphereIB;
        uint32_t sphereIndexCount;
        FGeometryUtils::CreateSphere(*m_device, sphereVB, sphereIB, sphereIndexCount);
        
        auto lightPassPtr = std::make_unique<FDeferredLightingPass>(m_device, m_gBuffer, m_shadowPass->GetDepthMap());
        m_lightingPass = lightPassPtr.get();
        // Set geometry data for light volumes
        m_lightingPass->SetSphereGeometry(sphereVB, sphereIB, sphereIndexCount);
        m_lightingPass->SetResolution(width, height);
        m_lightingPass->SetOutputTarget(m_hdrLightingFBO);
        
        m_renderGraph->AddPass(std::move(lightPassPtr));

        // 4. Forward Pass
        auto forwardPassPtr = std::make_unique<FForwardLitPass>();
        m_forwardPass = forwardPassPtr.get();
        m_forwardPass->SetOutputTarget(m_hdrForwardFBO);
        m_forwardPass->SetResolution(width, height);
        m_renderGraph->AddPass(std::move(forwardPassPtr));

        // 5. Post Process Pass
        auto postPassPtr = std::make_unique<FPostProcessPass>(m_device);
        postPassPtr->SetInputTexture(m_hdrLightingFBO->GetColorAttachment(0));
        postPassPtr->SetResolution(width, height);
        m_postProcessPass = postPassPtr.get();
        m_renderGraph->AddPass(std::move(postPassPtr));
    }

    void FSceneRenderer::CreateFBOs(uint32_t width, uint32_t height) {
        FFramebufferConfig gBufferConfig;
        gBufferConfig.Width = width;
        gBufferConfig.Height = height;
        gBufferConfig.DepthAttachment = m_device->CreateTexture(width, height, ERHIPixelFormat::D24_S8);
        gBufferConfig.ColorAttachments.push_back(m_device->CreateTexture(width, height, ERHIPixelFormat::RGBA8_UNORM)); // Albedo
        gBufferConfig.ColorAttachments.push_back(m_device->CreateTexture(width, height, ERHIPixelFormat::RGBA16_FLOAT)); // Normal
        gBufferConfig.ColorAttachments.push_back(m_device->CreateTexture(width, height, ERHIPixelFormat::RGBA8_UNORM)); // Emissive
        m_gBuffer = m_device->CreateFramebuffer(gBufferConfig);

        auto hdrColorTex = m_device->CreateTexture(width, height, ERHIPixelFormat::RGBA16_FLOAT);

        FFramebufferConfig hdrLightingConfig;
        hdrLightingConfig.Width = width;
        hdrLightingConfig.Height = height;
        hdrLightingConfig.ColorAttachments.push_back(hdrColorTex);
        m_hdrLightingFBO = m_device->CreateFramebuffer(hdrLightingConfig);

        FFramebufferConfig hdrForwardConfig;
        hdrForwardConfig.Width = width;
        hdrForwardConfig.Height = height;
        hdrForwardConfig.DepthAttachment = gBufferConfig.DepthAttachment;
        hdrForwardConfig.ColorAttachments.push_back(hdrColorTex);
        m_hdrForwardFBO = m_device->CreateFramebuffer(hdrForwardConfig);
    }

    void FSceneRenderer::Resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) return;
        if (width == m_width && height == m_height) return;
        m_width = width;
        m_height = height;
        
        CreateFBOs(width, height);
        
        if (m_postProcessPass) {
            m_postProcessPass->SetInputTexture(m_hdrLightingFBO->GetColorAttachment(0));
            m_postProcessPass->SetResolution(width, height);
        }
        
        if (m_lightingPass) {
            m_lightingPass->SetResolution(width, height);
            m_lightingPass->SetOutputTarget(m_hdrLightingFBO);
        }

        if (m_forwardPass) {
            m_forwardPass->SetResolution(width, height);
            m_forwardPass->SetOutputTarget(m_hdrForwardFBO);
        }
    }

    static void ResetRenderState(IRHICommandBuffer& cmdBuffer) {
        cmdBuffer.SetBlendState(false);
        cmdBuffer.SetDepthTest(true, true, ERHICompareFunc::LessEqual);
        cmdBuffer.SetCullMode(ERHICullMode::Back);
    }

    void FSceneRenderer::Render(const FRenderContext& context, 
                               const std::vector<FRenderable>& deferredList,
                               const std::vector<FRenderable>& forwardList) {
        
        for (auto& r : deferredList) {
            if (auto mat = std::dynamic_pointer_cast<FStandardPBRMaterial>(r.Material)) {
                mat->SetParameter("shadowMap", m_shadowPass->GetDepthMap());
            }
        }
        for (auto& r : forwardList) {
            if (auto mat = std::dynamic_pointer_cast<FStandardPBRMaterial>(r.Material)) {
                mat->SetParameter("shadowMap", m_shadowPass->GetDepthMap());
            }
        }

        m_cmdBuffer->Begin();

        std::vector<FRenderPass*> passes = m_renderGraph->GetPasses();

        ResetRenderState(*m_cmdBuffer);
        passes[0]->Execute(*m_cmdBuffer, context, deferredList);

        ResetRenderState(*m_cmdBuffer); 
        passes[1]->Execute(*m_cmdBuffer, context, deferredList);

        passes[2]->Execute(*m_cmdBuffer, context, deferredList);

        ResetRenderState(*m_cmdBuffer);
        passes[3]->Execute(*m_cmdBuffer, context, forwardList);

        // Final: Post Process -> Screen (target/state handled by pass)
        if (m_postProcessPass) {
            m_postProcessPass->Execute(*m_cmdBuffer, context, deferredList);
        }

        m_cmdBuffer->End();
        m_device->SubmitCommandBuffer(m_cmdBuffer);
    }

    void FSceneRenderer::Shutdown() {
    }

    std::shared_ptr<IRHITexture> FSceneRenderer::GetGBufferAlbedo() const { return m_gBuffer->GetColorAttachment(0); }
    std::shared_ptr<IRHITexture> FSceneRenderer::GetGBufferNormal() const { return m_gBuffer->GetColorAttachment(1); }
    std::shared_ptr<IRHITexture> FSceneRenderer::GetGBufferDepth() const { return m_gBuffer->GetDepthAttachment(); }
    std::shared_ptr<IRHITexture> FSceneRenderer::GetHDRColor() const { return m_hdrLightingFBO->GetColorAttachment(0); }

}
