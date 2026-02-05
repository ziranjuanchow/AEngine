#include "SandboxModule.h"
#include "Kernel/Core/Log.h"
#include "Kernel/ModuleManager/ModuleManager.h"
#include "Engine.Renderer/RenderModule.h"
#include "Engine.Window/WindowModule.h"
#include "Engine.Scene/GeometryUtils.h"
#include "Engine.Asset/AssetLoader.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <portable-file-dialogs.h>

#include <iostream>

namespace AEngine {

    void SandboxModule::OnStartup() {
        std::cout << "[SandboxModule] OnStartup Begin" << std::endl;
        AE_CORE_INFO("SandboxModule Starting...");

        auto* renderModule = FModuleManager::Get().GetModule<URenderModule>("Engine.Renderer");
        if (!renderModule) {
            AE_CORE_CRITICAL("RenderModule not found!");
            std::cout << "[SandboxModule] RenderModule Not Found!" << std::endl;
            return;
        }
        
        // 尝试获取设备（触发懒加载）
        m_device = renderModule->GetDevice();
        if (!m_device) {
            AE_CORE_CRITICAL("Failed to initialize RHI Device!");
            std::cout << "[SandboxModule] RHI Device Init Failed (GetDevice returned null)!" << std::endl;
            return;
        }
        std::cout << "[SandboxModule] Device Acquired." << std::endl;

        // 2. Load Resources (Fallback)
        FGeometryUtils::CreateSphere(*m_device, m_sphereVB, m_sphereIB, m_sphereIndexCount);

        std::shared_ptr<IRHIBuffer> quadVB, quadIB;
        uint32_t quadIndexCount;
        FGeometryUtils::CreateQuad(*m_device, quadVB, quadIB, quadIndexCount);
        std::cout << "[SandboxModule] Geometry Created." << std::endl;

        // 3. Material Setup
        m_pbrMat = std::make_shared<FStandardPBRMaterial>("StandardPBR");
        std::cout << "[SandboxModule] Loading Shaders..." << std::endl;
        m_pbrMat->LoadShaders("shaders/StandardPBR.vert", "shaders/StandardPBR.frag");
        std::cout << "[SandboxModule] Shaders Loaded." << std::endl;
        m_pbrMat->SetParameter("albedo", glm::vec3(1.0f, 1.0f, 1.0f)); // Changed to white for better visualization
        m_pbrMat->SetParameter("ao", 1.0f);

        // 4. Initial Scene Setup
        m_rootNode = std::make_unique<FSceneNode>("SceneRoot");
        // ... (rest of code)

        auto groundNode = std::make_unique<FSceneNode>("Ground");
        {
            FRenderable r;
            r.VertexBuffer = quadVB;
            r.IndexBuffer = quadIB;
            r.IndexCount = quadIndexCount;
            r.Material = m_pbrMat;
            groundNode->AddRenderable(r);
        }
        m_rootNode->AddChild(std::move(groundNode));

        SetupScene();
    }

    void SandboxModule::OnShutdown() {
        m_rootNode.reset();
        m_pbrMat.reset();
        m_sphereVB.reset();
        m_sphereIB.reset();
        m_device.reset();
    }

    void SandboxModule::SetupScene() {
        for (int i = 0; i < 5; ++i) {
            auto sphereNode = std::make_unique<FSceneNode>("Sphere_" + std::to_string(i));
            sphereNode->SetPosition(glm::vec3(i * 2.5f - 5.0f, 0.0f, 0.0f));
            FRenderable r;
            r.VertexBuffer = m_sphereVB;
            r.IndexBuffer = m_sphereIB;
            r.IndexCount = m_sphereIndexCount;
            r.Material = m_pbrMat;
            sphereNode->AddRenderable(r);
            m_rootNode->AddChild(std::move(sphereNode));
        }

        auto lightRoot = std::make_unique<FSceneNode>("Lights");
        for (int i = 0; i < 50; ++i) {
            FPointLight light;
            light.Position = glm::vec3(
                ((rand() % 100) / 100.0f) * 40.0f - 20.0f,
                ((rand() % 100) / 100.0f) * 10.0f,
                ((rand() % 100) / 100.0f) * 40.0f - 20.0f
            );
            light.Color = glm::vec3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
            light.Intensity = 10.0f;
            light.Radius = 10.0f;

            auto lightNode = std::make_unique<FSceneNode>("PointLight_" + std::to_string(i));
            lightNode->SetPosition(light.Position);
            lightNode->SetPointLight(light);
            lightNode->SetRenderPassType(ERenderPassType::Forward);
            
            FRenderable debugSphere;
            debugSphere.VertexBuffer = m_sphereVB;
            debugSphere.IndexBuffer = m_sphereIB;
            debugSphere.IndexCount = m_sphereIndexCount;
            debugSphere.Material = m_pbrMat;
            debugSphere.WorldMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)); 
            lightNode->AddRenderable(debugSphere);

            lightRoot->AddChild(std::move(lightNode));
        }
        m_rootNode->AddChild(std::move(lightRoot));
    }

    void SandboxModule::OnUpdate(float deltaTime) {
        // 1. Process Input
        auto* windowMod = FModuleManager::Get().GetModule<UWindowModule>("Engine.Window");
        if (windowMod) {
            GLFWwindow* window = windowMod->GetNativeWindow();
            
            float speed = 5.0f * deltaTime;
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speed *= 2.0f;

            glm::vec3 front;
            front.x = cos(glm::radians(m_cameraYaw)) * cos(glm::radians(m_cameraPitch));
            front.y = sin(glm::radians(m_cameraPitch));
            front.z = sin(glm::radians(m_cameraYaw)) * cos(glm::radians(m_cameraPitch));
            glm::vec3 cameraFront = glm::normalize(front);
            glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));
            glm::vec3 cameraUp    = glm::normalize(glm::cross(cameraRight, cameraFront));

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) m_cameraPos += cameraFront * speed;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) m_cameraPos -= cameraFront * speed;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) m_cameraPos -= cameraRight * speed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) m_cameraPos += cameraRight * speed;

            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                if (m_firstMouse) { m_lastX = xpos; m_lastY = ypos; m_firstMouse = false; }
                float xoffset = (float)(xpos - m_lastX);
                float yoffset = (float)(m_lastY - ypos); 
                m_lastX = xpos; m_lastY = ypos;
                float sensitivity = 0.05f;
                m_cameraYaw += xoffset * sensitivity;
                m_cameraPitch += yoffset * sensitivity;
                if(m_cameraPitch > 89.0f) m_cameraPitch = 89.0f;
                if(m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                m_firstMouse = true;
            }
            
            // 2. Render Request
            auto* renderModule = FModuleManager::Get().GetModule<URenderModule>("Engine.Renderer");
            if (renderModule) {
                int w, h;
                glfwGetFramebufferSize(window, &w, &h);
                renderModule->Resize(w, h);

                FRenderContext ctx;
                ctx.ViewMatrix = glm::lookAt(m_cameraPos, m_cameraPos + cameraFront, cameraUp);
                ctx.ProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100.0f);
                ctx.CameraPosition = m_cameraPos;
                ctx.LightPosition = glm::vec3(10.0f, 10.0f, 10.0f);
                ctx.LightColor = glm::vec3(150.0f, 150.0f, 150.0f);

                float near_plane = 1.0f, far_plane = 50.0f;
                glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
                glm::mat4 lightView = glm::lookAt(ctx.LightPosition, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
                ctx.LightSpaceMatrix = lightProjection * lightView;
                
                m_rootNode->UpdateWorldMatrix();
                m_deferredList.clear();
                m_forwardList.clear();
                m_pointLights.clear();
                CollectRenderables(m_rootNode.get());
                ctx.PointLights = m_pointLights;

                renderModule->Render(ctx, m_deferredList, m_forwardList);
            }
        }

        // 3. UI
        ImGui::Begin("Scene Hierarchy");
        if (m_rootNode) DrawSceneNode(m_rootNode.get());
        ImGui::End();

        ImGui::Begin("Inspector");
        if (m_selectedNode) {
            ImGui::Text("Name: %s", m_selectedNode->GetName().c_str());
            // ... (Simplified Transform Edit) ...
            glm::vec3 pos = m_selectedNode->GetPosition();
            if (ImGui::DragFloat3("Position", &pos[0], 0.1f)) m_selectedNode->SetPosition(pos);
        }
        ImGui::End();
        
        // Execute deferred actions
        for (auto& action : m_deferredActions) action();
        m_deferredActions.clear();
    }

    void SandboxModule::CollectRenderables(FSceneNode* node) {
        if (!node->IsVisible()) return;
        for (auto renderable : node->GetRenderables()) {
            renderable.WorldMatrix = node->GetWorldMatrix();
            if (node->GetRenderPassType() == ERenderPassType::Deferred) m_deferredList.push_back(renderable);
            else m_forwardList.push_back(renderable);
        }
        if (node->HasPointLight()) {
            FPointLight light = node->GetPointLight().value();
            light.Position = glm::vec3(node->GetWorldMatrix()[3]);
            m_pointLights.push_back(light);
        }
        for (const auto& child : node->GetChildren()) CollectRenderables(child.get());
    }

    void SandboxModule::DrawSceneNode(FSceneNode* node) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (node == m_selectedNode) flags |= ImGuiTreeNodeFlags_Selected;
        if (node->GetChildren().empty()) flags |= ImGuiTreeNodeFlags_Leaf;
        
        bool opened = ImGui::TreeNodeEx((void*)node, flags, node->GetName().c_str());
        if (ImGui::IsItemClicked()) m_selectedNode = node;
        if (opened) {
            for (const auto& child : node->GetChildren()) DrawSceneNode(child.get());
            ImGui::TreePop();
        }
    }

}

AE_IMPLEMENT_MODULE(AEngine::SandboxModule)
