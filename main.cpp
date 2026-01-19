#include <iostream>
#include <glad/glad.h>
#include "Core/Log.h"
#include "Core/Engine.h"
#include "Core/WindowSubsystem.h"
#include "Core/PluginManager.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>

int main() {
    using namespace AEngine;

    Log::Init();
    AE_CORE_INFO("AEngine Initialized!");

    auto& engine = UEngine::Get();
    
    // Register subsystems
    auto windowSubsystem = std::make_unique<UWindowSubsystem>();
    UWindowSubsystem* windowPtr = windowSubsystem.get();
    if (auto result = engine.RegisterSubsystem(std::move(windowSubsystem)); !result) {
        AE_CORE_ERROR("Failed to register WindowSubsystem!");
        return -1;
    }

    engine.Init();

    // Load first plugin
    auto& pluginManager = APluginManager::Get();
    if (auto loadResult = pluginManager.LoadPlugin("StatsPanel.dll"); !loadResult) {
        AE_CORE_ERROR("Failed to load StatsPanel plugin!");
    }

    while (!windowPtr->ShouldClose()) {
        windowPtr->Update(); // Poll events, ImGui NewFrame

        // UI Rendering
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // Render loaded plugins
        if (auto* stats = pluginManager.GetPlugin("StatsPanel")) {
            stats->OnImGuiRender();
        }
        
        // Manual ImGui Demo
        // ImGui::ShowDemoWindow();

        ImGui::Render();
        
        int display_w, display_h;
        glfwGetFramebufferSize(windowPtr->GetNativeWindow(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows (Docking)
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(windowPtr->GetNativeWindow());
    }

    engine.Shutdown();
    return 0;
}