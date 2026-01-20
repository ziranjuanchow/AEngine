#include <iostream>
#include <glad/glad.h>
#include "Core/Log.h"
#include "Core/Engine.h"
#include "Core/WindowSubsystem.h"
#include "Core/PluginManager.h"
#include "Core/GeometryUtils.h"
#include "RHI/OpenGL/OpenGLDevice.h"
#include "RHI/ForwardLitPass.h"
#include "RHI/StandardPBRMaterial.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>

int main() {
    using namespace AEngine;

    Log::Init();
    AE_CORE_INFO("AEngine PBR Demo Starting...");

    auto& engine = UEngine::Get();
    auto windowSubsystem = std::make_unique<UWindowSubsystem>();
    UWindowSubsystem* windowPtr = windowSubsystem.get();
    engine.RegisterSubsystem(std::move(windowSubsystem));
    engine.Init();

    // RHI & Pipeline Setup
    auto device = std::make_shared<FOpenGLDevice>();
    auto renderGraph = std::make_unique<FRenderGraph>();
    renderGraph->AddPass(std::make_unique<FForwardLitPass>());

    // Material Setup
    auto pbrMat = std::make_shared<FStandardPBRMaterial>("StandardPBR");
    pbrMat->LoadShaders("shaders/StandardPBR.vert", "shaders/StandardPBR.frag");
    pbrMat->SetParameter("albedo", glm::vec3(0.5f, 0.0f, 0.0f)); // Red
    pbrMat->SetParameter("ao", 1.0f);

    // Geometry Setup
    std::shared_ptr<IRHIBuffer> sphereVB, sphereIB;
    uint32_t sphereIndexCount;
    FGeometryUtils::CreateSphere(*device, sphereVB, sphereIB, sphereIndexCount);

    // Scene Setup (Manual for now)
    std::vector<FRenderable> scene;
    for (int i = 0; i < 5; ++i) {
        FRenderable r;
        r.VertexBuffer = sphereVB;
        r.IndexBuffer = sphereIB;
        r.IndexCount = sphereIndexCount;
        r.Material = pbrMat;
        r.WorldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i * 2.5f - 5.0f, 0.0f, 0.0f));
        scene.push_back(r);
    }

    auto cmdBuffer = device->CreateCommandBuffer();

    while (!windowPtr->ShouldClose()) {
        windowPtr->Update();

        // UI
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::Begin("Material Editor");
        static float metallic = 0.5f;
        static float roughness = 0.5f;
        ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
        pbrMat->SetParameter("metallic", metallic);
        pbrMat->SetParameter("roughness", roughness);
        ImGui::End();

        // Render
        int w, h;
        glfwGetFramebufferSize(windowPtr->GetNativeWindow(), &w, &h);
        
        FRenderContext ctx;
        ctx.ViewMatrix = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        ctx.ProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100.0f);
        ctx.CameraPosition = glm::vec3(0, 0, 10);
        ctx.LightPosition = glm::vec3(10.0f, 10.0f, 10.0f);
        ctx.LightColor = glm::vec3(150.0f, 150.0f, 150.0f); // Bright light

        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        cmdBuffer->Begin();
        renderGraph->Execute(*cmdBuffer, ctx, scene);
        cmdBuffer->End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup);
        }

        glfwSwapBuffers(windowPtr->GetNativeWindow());
    }

    engine.Shutdown();
    
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    return 0;
}
