#include "FApplication.h"
#include "Kernel/Core/Log.h"
#include "Kernel/ModuleManager/ModuleManager.h"
#include "Engine.Window/WindowModule.h"
#include "Engine.Renderer/RenderModule.h"
// 在 Phase 0/1 混合期，为了静态链接，我们需要引用 RHIModule 的实现来注册它
// 但理想情况下，我们不应该引用 Engine/Plugins 下的 cpp 文件
// 我们可以声明一个外部函数，或者更优雅地，把 CreateModule 导出
namespace AEngine { extern IModule* CreateOpenGLModule(); } 

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

namespace AEngine {

    FApplication::FApplication(const FApplicationSpecs& specs)
        : m_specs(specs) {
        
        Log::Init();
        AE_CORE_INFO("Initializing Modular Application: {0}", specs.Name);

        auto& mm = FModuleManager::Get();
        
        // Phase 0: 静态注册模块
        mm.RegisterStaticModule("Engine.Window", []() { return std::make_unique<UWindowModule>(); });
        mm.RegisterStaticModule("Engine.Renderer", []() { return std::make_unique<URenderModule>(); });

        // 扫描并解析
        mm.DiscoverModules("src/Engine/Modules");
        mm.DiscoverModules("src/Engine/Plugins");
        
        // 启用模块：Window, Renderer 以及具体的 RHI 实现 (OpenGL)
        if (!mm.ResolveDependencies({"Engine.Window", "Engine.Renderer", "RHI.OpenGL"})) {
            AE_CORE_CRITICAL("Failed to resolve module dependencies! Application cannot start.");
            m_running = false;
            return;
        }
        mm.StartupModules();
    }

    FApplication::~FApplication() {
        FModuleManager::Get().ShutdownModules();
    }

    void FApplication::Run() {
        if (!m_running) return;

        auto* windowModule = FModuleManager::Get().GetModule<UWindowModule>("Engine.Window");
        if (!windowModule) {
            AE_CORE_CRITICAL("WindowModule not found! Cannot run application.");
            return;
        }

        OnInit();

        while (m_running) {
            if (windowModule->ShouldClose()) {
                m_running = false;
                continue;
            }

            // 更新所有模块（包括 WindowModule 的 PollEvents 和 NewFrame）
            FModuleManager::Get().UpdateModules(0.016f); 

            OnUpdate(0.016f);

            // ImGui Rendering
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
            
            OnImGuiRender();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* backup = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup);
            }

            glfwSwapBuffers(windowModule->GetNativeWindow());
        }

        OnShutdown();
    }

}
