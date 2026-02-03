#include "SceneRenderer.h"
#include "ShadowPass.h"
#include "DeferredGeometryPass.h"
#include "DeferredLightingPass.h"
#include "ForwardLitPass.h"
#include "PostProcessPass.h"
#include "StandardPBRMaterial.h"
#include "Kernel/Core/Log.h"
#include "../Core/GeometryUtils.h"
#include <glad/glad.h>

namespace AEngine {

    FSceneRenderer::FSceneRenderer(std::shared_ptr<IRHIDevice> device)
        : m_device(device) {
        m_renderGraph = std::make_unique<FRenderGraph>();
        m_cmdBuffer = m_device->CreateCommandBuffer();
    }

    FSceneRenderer::~FSceneRenderer() {
        Shutdown();
    }

    /// @brief Initializes the renderer, including FBOs and the Render Graph.
    /// @param width Viewport width
    /// @param height Viewport height
    /// The initialization pipeline currently hardcodes the following passes:
    /// 1. Shadow Pass: Generates shadow maps from directional lights.
    /// 2. G-Buffer Pass: Deferred geometry pass (MRT: Albedo, Normal, Emissive).
    /// 3. Lighting Pass: Deferred lighting using light volumes (spheres).
    /// 4. Forward Pass: For transparent objects, skybox, or debug lines (reuses Depth/Color).
    /// 5. Post Process Pass: Tone mapping and gamma correction.
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
        auto shadowPassPtr = std::make_unique<FShadowPass>(shadowFBO);
        m_shadowPass = shadowPassPtr.get();
        m_renderGraph->AddPass(std::move(shadowPassPtr));

        // 2. Deferred Geometry Pass
        auto geomPassPtr = std::make_unique<FDeferredGeometryPass>(m_gBuffer);
        m_geometryPass = geomPassPtr.get();
        m_renderGraph->AddPass(std::move(geomPassPtr));

        // 3. Deferred Lighting Pass
        // We use a unit sphere for light volume rendering.
        // The sphere is scaled by the light radius in the shader.
        std::shared_ptr<IRHIBuffer> sphereVB, sphereIB;
        uint32_t sphereIndexCount;
        FGeometryUtils::CreateSphere(*m_device, sphereVB, sphereIB, sphereIndexCount);
        
        auto lightPassPtr = std::make_unique<FDeferredLightingPass>(m_gBuffer, sphereVB, sphereIB, sphereIndexCount, width, height);
        m_lightingPass = lightPassPtr.get();
        m_renderGraph->AddPass(std::move(lightPassPtr));

        // 4. Forward Pass
        auto forwardPassPtr = std::make_unique<FForwardLitPass>();
        m_forwardPass = forwardPassPtr.get();
        m_renderGraph->AddPass(std::move(forwardPassPtr));

        // 5. Post Process Pass
        auto postPassPtr = std::make_unique<FPostProcessPass>(m_hdrLightingFBO->GetColorAttachment(0));
        m_postProcessPass = postPassPtr.get();
        // PostProcess is usually the final step, but we don't strictly add it to graph if we want manual control
        // But we MUST keep it alive! So add it to graph.
        m_renderGraph->AddPass(std::move(postPassPtr));
    }

    /// @brief Creates Framebuffers for G-Buffer and HDR Lighting.
    /// G-Buffer Layout:
    /// - Attachment 0: Albedo (RGB) + Specular (A) [RGBA8_UNORM]
    /// - Attachment 1: Normal (RGB) [RGBA16_FLOAT for precision]
    /// - Attachment 2: Emissive / Other data [RGBA8_UNORM]
    /// - Depth: D24_S8 (Shared with Forward Pass)
    void FSceneRenderer::CreateFBOs(uint32_t width, uint32_t height) {
        // G-Buffer Setup
        FFramebufferConfig gBufferConfig;
        gBufferConfig.Width = width;
        gBufferConfig.Height = height;
        gBufferConfig.DepthAttachment = m_device->CreateTexture(width, height, ERHIPixelFormat::D24_S8);
        gBufferConfig.ColorAttachments.push_back(m_device->CreateTexture(width, height, ERHIPixelFormat::RGBA8_UNORM)); // Albedo
        gBufferConfig.ColorAttachments.push_back(m_device->CreateTexture(width, height, ERHIPixelFormat::RGBA16_FLOAT)); // Normal
        gBufferConfig.ColorAttachments.push_back(m_device->CreateTexture(width, height, ERHIPixelFormat::RGBA8_UNORM)); // Emissive
        m_gBuffer = m_device->CreateFramebuffer(gBufferConfig);

        // HDR FBOs Setup
        auto hdrColorTex = m_device->CreateTexture(width, height, ERHIPixelFormat::RGBA16_FLOAT);

        FFramebufferConfig hdrLightingConfig;
        hdrLightingConfig.Width = width;
        hdrLightingConfig.Height = height;
        hdrLightingConfig.ColorAttachments.push_back(hdrColorTex);
        m_hdrLightingFBO = m_device->CreateFramebuffer(hdrLightingConfig);

        FFramebufferConfig hdrForwardConfig;
        hdrForwardConfig.Width = width;
        hdrForwardConfig.Height = height;
        hdrForwardConfig.DepthAttachment = gBufferConfig.DepthAttachment; // Shared Depth
        hdrForwardConfig.ColorAttachments.push_back(hdrColorTex);         // Shared Color
        m_hdrForwardFBO = m_device->CreateFramebuffer(hdrForwardConfig);
    }

    void FSceneRenderer::Resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0) return;
        if (width == m_width && height == m_height) return;
        m_width = width;
        m_height = height;
        
        CreateFBOs(width, height);
        
        // Update texture references in passes
        if (m_postProcessPass) {
            m_postProcessPass->SetInputTexture(m_hdrLightingFBO->GetColorAttachment(0));
        }
        
        if (m_lightingPass) {
            m_lightingPass->SetResolution(width, height);
        }
    }

    static void UnbindAllTextures() {
        for (int i = 0; i < 8; ++i) {
            // TODO: Refactor to RHI (m_cmdBuffer->ResetTextureUnits())
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }

    /// @brief Resets common OpenGL render states to a known default (Depth On, Backface Culling).
    static void ResetRenderState() {
        // TODO: Refactor to RHI (m_cmdBuffer->SetRenderState(...))
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    /// @brief Executes the full rendering pipeline.
    /// Pipeline Flow:
    /// 1. Shadow Pass: Render depth from light perspective.
    /// 2. G-Buffer Pass: Fill Albedo, Normal, Depth.
    /// 3. Lighting Pass: Accumulate lighting into HDR buffer using G-Buffer.
    /// 4. Forward Pass: Draw transparent/debug objects on top of HDR buffer (using G-Buffer depth).
    /// 5. Post Process: Tone map HDR -> LDR (Screen).
    void FSceneRenderer::Render(const FRenderContext& context, 
                               const std::vector<FRenderable>& deferredList,
                               const std::vector<FRenderable>& forwardList) {
        
        // 1. Prepare Materials (Shadow Map Binding)
        // This iterates through all renderables to bind the shadow map texture.
        // TODO: This should be handled by a global uniform buffer or resource binding system.
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

        // ---------------------------------------------------------
        // Pass 0: Shadow (Location 0 in Graph)
        // ---------------------------------------------------------
        // Shadow Pass manages its own state (Cull Front), but we should ensure a clean start
        ResetRenderState();
        passes[0]->Execute(*m_cmdBuffer, context, deferredList);
        UnbindAllTextures();

        // ---------------------------------------------------------
        // Pass 1: Geometry (Location 1 in Graph)
        // ---------------------------------------------------------
        m_gBuffer->Bind();
        m_cmdBuffer->SetViewport(0, 0, m_width, m_height);
        m_cmdBuffer->Clear(0.0f, 0.0f, 0.0f, 1.0f); 
        ResetRenderState(); // CRITICAL: Reset Cull Mode back to BACK after Shadow Pass!
        passes[1]->Execute(*m_cmdBuffer, context, deferredList);
        m_gBuffer->Unbind();
        UnbindAllTextures();

        // ---------------------------------------------------------
        // Pass 2: Lighting (Location 2 in Graph)
        // ---------------------------------------------------------
        m_hdrLightingFBO->Bind();
        m_cmdBuffer->SetViewport(0, 0, m_width, m_height);
        m_cmdBuffer->Clear(0.0f, 0.0f, 0.0f, 1.0f, false); // Clear Color Only
        // Lighting pass sets its own state (Blend, No Depth, Cull Front) within Execute()
        passes[2]->Execute(*m_cmdBuffer, context, deferredList);
        m_hdrLightingFBO->Unbind();
        UnbindAllTextures();

        // ---------------------------------------------------------
        // Pass 3: Forward (Location 3 in Graph)
        // ---------------------------------------------------------
        // Note: We bind the HDR FBO again, which shares Depth with G-Buffer.
        m_hdrForwardFBO->Bind();
        m_cmdBuffer->SetViewport(0, 0, m_width, m_height);
        ResetRenderState(); // Reset for Forward Pass (Depth Test On, Cull Back)
        // Do not clear! We draw on top of lighting results using G-Buffer depth
        passes[3]->Execute(*m_cmdBuffer, context, forwardList);
        m_hdrForwardFBO->Unbind();
        UnbindAllTextures();

        // ---------------------------------------------------------
        // Final: Post Process -> Screen
        // ---------------------------------------------------------
        // TODO: Refactor to RHI (m_cmdBuffer->BindBackBuffer())
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_width, m_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ResetRenderState(); // Reset for Post Process (though it usually disables depth)

        if (m_postProcessPass) {
            m_postProcessPass->Execute(*m_cmdBuffer, context, deferredList);
        }
        UnbindAllTextures();

        m_cmdBuffer->End();
    }

    void FSceneRenderer::Shutdown() {
        // Cleanup if needed
    }

    std::shared_ptr<IRHITexture> FSceneRenderer::GetGBufferAlbedo() const { return m_gBuffer->GetColorAttachment(0); }
    std::shared_ptr<IRHITexture> FSceneRenderer::GetGBufferNormal() const { return m_gBuffer->GetColorAttachment(1); }
    std::shared_ptr<IRHITexture> FSceneRenderer::GetGBufferDepth() const { return m_gBuffer->GetDepthAttachment(); }
    std::shared_ptr<IRHITexture> FSceneRenderer::GetHDRColor() const { return m_hdrLightingFBO->GetColorAttachment(0); }

}
