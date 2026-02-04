#include "RenderModule.h"
#include "Kernel/ModuleManager/ModuleManager.h"
#include "Kernel/Core/Log.h"
#include "Engine.Window/WindowModule.h"
#include "Engine.RHI/RHIModule.h"
#include <GLFW/glfw3.h> // For glfwGetFramebufferSize

namespace AEngine {

    void URenderModule::OnStartup() {
        AE_CORE_INFO("RenderModule starting...");
        TryInitDevice();
    }

    void URenderModule::OnShutdown() {
        m_renderer.reset();
        m_device.reset();
    }

    bool URenderModule::TryInitDevice() {
        if (m_device) return true;

        auto* rhiModule = FModuleManager::Get().GetModule<IRHIModule>("RHI.OpenGL");
        if (!rhiModule) {
             // AE_CORE_WARN("RenderModule: RHI.OpenGL module not ready yet.");
             return false;
        }
        
        AE_CORE_INFO("RenderModule: Initializing Device from RHI.OpenGL");
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
        return true;
    }

    void URenderModule::Render(const FRenderContext& ctx, 
                               const std::vector<FRenderable>& deferred, 
                               const std::vector<FRenderable>& forward) {
        if (!m_device) {
            if (!TryInitDevice()) return;
        }

        if (m_renderer) {
            m_renderer->Render(ctx, deferred, forward);
        }
    }

    void URenderModule::Resize(uint32_t width, uint32_t height) {
        if (!m_device) return;
        
        if (m_renderer) {
            m_renderer->Resize(width, height);
        }
    }
}