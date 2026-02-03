#include "RenderModule.h"
#include "Kernel/ModuleManager/ModuleManager.h"
#include "Kernel/Core/Log.h"
#include "Engine.Window/WindowModule.h"
#include "Engine.RHI/RHIModule.h"

namespace AEngine {

    void URenderModule::OnStartup() {
        AE_CORE_INFO("RenderModule starting...");
        
        auto* rhiModule = FModuleManager::Get().GetModule<IRHIModule>("RHI.OpenGL");
        if (!rhiModule) {
             AE_CORE_CRITICAL("RenderModule: RHI.OpenGL module not found! Ensure it is enabled in config.");
             return; // Or fallback
        }
        m_device = rhiModule->CreateDevice();
        m_renderer = std::make_unique<FSceneRenderer>(m_device);

        auto* windowMod = FModuleManager::Get().GetModule<UWindowModule>("Engine.Window");
        if (windowMod) {
             int w, h;
             glfwGetFramebufferSize(windowMod->GetNativeWindow(), &w, &h);
             m_renderer->Init(w, h);
        } else {
             AE_CORE_ERROR("RenderModule: Could not find WindowModule!");
             m_renderer->Init(1280, 720);
        }
    }

    void URenderModule::OnShutdown() {
        m_renderer.reset();
        m_device.reset();
    }

    void URenderModule::Render(const FRenderContext& ctx, 
                               const std::vector<FRenderable>& deferred, 
                               const std::vector<FRenderable>& forward) {
        if (m_renderer) {
            m_renderer->Render(ctx, deferred, forward);
        }
    }

    void URenderModule::Resize(uint32_t width, uint32_t height) {
        if (m_renderer) {
            m_renderer->Resize(width, height);
        }
    }
}
