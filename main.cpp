#include <iostream>
#include <glad/glad.h>
#include "Core/FApplication.h"
#include "Core/Log.h"
#include "Core/PluginManager.h"
#include "Core/GeometryUtils.h"
#include "Core/AssetLoader.h"
#include "RHI/OpenGL/OpenGLDevice.h"
#include "RHI/ForwardLitPass.h"
#include "RHI/StandardPBRMaterial.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <portable-file-dialogs.h>

using namespace AEngine;

class SandboxApp : public FApplication {
public:
    SandboxApp() : FApplication({ "AEngine Sandbox", 1600, 900 }) {}

    virtual void OnInit() override {
        AE_CORE_INFO("SandboxApp Init");

        // Load Plugin (Optional)
        auto& pluginManager = APluginManager::Get();
        if (auto loadResult = pluginManager.LoadPlugin("StatsPanel.dll"); !loadResult) {
            AE_CORE_ERROR("Failed to load StatsPanel plugin!");
        }

        // RHI & Pipeline Setup
        m_device = std::make_shared<FOpenGLDevice>();
        m_renderGraph = std::make_unique<FRenderGraph>();
        m_renderGraph->AddPass(std::make_unique<FForwardLitPass>());

        // Material Setup
        m_pbrMat = std::make_shared<FStandardPBRMaterial>("StandardPBR");
        m_pbrMat->LoadShaders("shaders/StandardPBR.vert", "shaders/StandardPBR.frag");
        m_pbrMat->SetParameter("albedo", glm::vec3(0.5f, 0.0f, 0.0f)); // Red
        m_pbrMat->SetParameter("ao", 1.0f);

        // Geometry Setup (Fallback)
        uint32_t sphereIndexCount;
        FGeometryUtils::CreateSphere(*m_device, m_sphereVB, m_sphereIB, sphereIndexCount);

        // Initial Scene Setup
        for (int i = 0; i < 5; ++i) {
            FRenderable r;
            r.VertexBuffer = m_sphereVB;
            r.IndexBuffer = m_sphereIB;
            r.IndexCount = sphereIndexCount;
            r.Material = m_pbrMat;
            r.WorldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i * 2.5f - 5.0f, 0.0f, 0.0f));
            m_scene.push_back(r);
        }

        m_cmdBuffer = m_device->CreateCommandBuffer();
    }

    virtual void OnUpdate(float deltaTime) override {
        // Clear Screen
        int w, h;
        glfwGetFramebufferSize(m_window->GetNativeWindow(), &w, &h);
        
        if (w <= 0 || h <= 0) return; // Window minimized or invalid size

        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Prepare Render Context
        FRenderContext ctx;
        ctx.ViewMatrix = glm::lookAt(glm::vec3(0, 0, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        ctx.ProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100.0f);
        ctx.CameraPosition = glm::vec3(0, 0, 10);
        ctx.LightPosition = glm::vec3(10.0f, 10.0f, 10.0f);
        ctx.LightColor = glm::vec3(150.0f, 150.0f, 150.0f);

        // Execute Render Graph
        m_cmdBuffer->Begin();
        m_renderGraph->Execute(*m_cmdBuffer, ctx, m_scene);
        m_cmdBuffer->End();
    }

    virtual void OnImGuiRender() override {
        auto& pluginManager = APluginManager::Get();
        if (auto* stats = pluginManager.GetPlugin("StatsPanel")) {
            stats->OnImGuiRender();
        }

        ImGui::Begin("Asset Loader");
        static char modelPath[256] = "";
        ImGui::InputText("Model Path", modelPath, 256);
        
        if (ImGui::Button("Browse...")) {
            auto selection = pfd::open_file("Select a 3D Model", ".",
                                { "3D Models", "*.obj *.fbx *.gltf *.glb",
                                  "All Files", "*" }).result();
            if (!selection.empty()) {
                strncpy(modelPath, selection[0].c_str(), 256);
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Load Model")) {
            if (auto newModel = FAssetLoader::LoadModel(*m_device, modelPath)) {
                m_currentModel = newModel;
                m_scene = m_currentModel->Renderables; 
            }
        }
        ImGui::End();

        ImGui::Begin("Material Editor");
        static float metallic = 0.5f;
        static float roughness = 0.5f;
        
        bool updateMat = false;
        if (ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f)) updateMat = true;
        if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f)) updateMat = true;
        
        if (updateMat) {
            // Update default material
            m_pbrMat->SetParameter("metallic", metallic);
            m_pbrMat->SetParameter("roughness", roughness);

            // Update loaded model materials
            if (m_currentModel) {
                for (auto& mesh : m_currentModel->Meshes) {
                    if (mesh.Material) {
                        mesh.Material->SetParameter("metallic", metallic);
                        mesh.Material->SetParameter("roughness", roughness);
                    }
                }
            }
        }
        ImGui::End();
    }

private:
    std::shared_ptr<FOpenGLDevice> m_device;
    std::unique_ptr<FRenderGraph> m_renderGraph;
    std::shared_ptr<IRHICommandBuffer> m_cmdBuffer;
    
    std::shared_ptr<FStandardPBRMaterial> m_pbrMat;
    std::shared_ptr<IRHIBuffer> m_sphereVB, m_sphereIB;
    
    std::vector<FRenderable> m_scene;
    std::shared_ptr<FModel> m_currentModel;
};

AEngine::FApplication* AEngine::CreateApplication() {
    return new SandboxApp();
}

int main() {
    auto* app = AEngine::CreateApplication();
    app->Run();
    delete app;
    return 0;
}