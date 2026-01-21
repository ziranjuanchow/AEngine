#include "FApplication.h"
#include "Log.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

namespace AEngine {

    FApplication::FApplication(const FApplicationSpecs& specs)
        : m_specs(specs) {
        
        Log::Init();
        AE_CORE_INFO("Initializing Application: {0}", specs.Name);

        auto& engine = UEngine::Get();
        
        // Register Window Subsystem
        auto windowSubsystem = std::make_unique<UWindowSubsystem>();
        // Configure window props if UWindowSubsystem supported it (TODO)
        m_window = windowSubsystem.get();
        
        if (auto result = engine.RegisterSubsystem(std::move(windowSubsystem)); !result) {
            AE_CORE_CRITICAL("Failed to register WindowSubsystem!");
            m_running = false;
            return;
        }

        engine.Init();
    }

    FApplication::~FApplication() {
        UEngine::Get().Shutdown();
    }

    void FApplication::Run() {
        if (!m_running) return;

        OnInit();

        while (m_running) {
            if (m_window->ShouldClose()) {
                m_running = false;
                continue;
            }

            m_window->Update(); // Poll events, NewFrame

            // Start ImGui Frame (Already started in WindowSubsystem::Update, but we structure it here logically)
            
            // Core Rendering / User Update
            // TODO: Calculate DeltaTime
            OnUpdate(0.016f);

            // ImGui Rendering
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
            
            OnImGuiRender();

            ImGui::Render();
            
            // Note: Clear logic should ideally be in RHI or Renderer, but for now we rely on the App/User to clear 
            // or the WindowSubsystem to handle swap.
            // Wait, WindowSubsystem doesn't Clear. The User OnUpdate is responsible for RHI commands.
            
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* backup = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup);
            }

            glfwSwapBuffers(m_window->GetNativeWindow());
        }

        OnShutdown();
    }

}
