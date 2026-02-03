#include <iostream>
#include <glad/glad.h>
#include "Core/FApplication.h"
#include "Kernel/Core/Log.h"
#include "Kernel/ModuleManager/ModuleManager.h"
#include "Engine.Window/WindowModule.h"
#include "Engine.Renderer/RenderModule.h"
#include "Engine.Scene/SceneNode.h"
#include "Core/PluginManager.h"
#include "Engine.Scene/GeometryUtils.h"
#include "Engine.Asset/AssetLoader.h"
#include "Engine.Renderer/StandardPBRMaterial.h"
#include "Engine.Renderer/PostProcessPass.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <portable-file-dialogs.h>
#include <functional>

using namespace AEngine;

class SandboxApp : public FApplication {
public:
    SandboxApp() : FApplication({ .Name = "AEngine Sandbox", .Width = 1600, .Height = 900 }) {
    }

    virtual void OnInit() override {
        AE_CORE_INFO("SandboxApp Init");

        auto* renderModule = FModuleManager::Get().GetModule<URenderModule>("Engine.Renderer");
        if (!renderModule) {
            AE_CORE_CRITICAL("RenderModule not found!");
            return;
        }
        auto device = renderModule->GetDevice();

        // 2. Load Resources (Fallback)
        uint32_t sphereIndexCount;
        FGeometryUtils::CreateSphere(*device, m_sphereVB, m_sphereIB, sphereIndexCount);
        m_sphereIndexCount = sphereIndexCount;

        std::shared_ptr<IRHIBuffer> quadVB, quadIB;
        uint32_t quadIndexCount;
        FGeometryUtils::CreateQuad(*device, quadVB, quadIB, quadIndexCount);

        // 3. Material Setup
        m_pbrMat = std::make_shared<FStandardPBRMaterial>("StandardPBR");
        m_pbrMat->LoadShaders("shaders/StandardPBR.vert", "shaders/StandardPBR.frag");
        m_pbrMat->SetParameter("albedo", glm::vec3(0.5f, 0.0f, 0.0f));
        m_pbrMat->SetParameter("ao", 1.0f);

        // 4. Initial Scene Setup
        m_rootNode = std::make_unique<FSceneNode>("SceneRoot");

        // Ground
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

        // Initial Spheres
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

        // 5. Point Lights Setup
        auto lightRoot = std::make_unique<FSceneNode>("Lights");
        for (int i = 0; i < 50; ++i) {
            FPointLight light;
            light.Position = glm::vec3(
                ((rand() % 100) / 100.0f) * 40.0f - 20.0f,
                ((rand() % 100) / 100.0f) * 10.0f,
                ((rand() % 100) / 100.0f) * 40.0f - 20.0f
            );
            light.Color = glm::vec3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f, (rand() % 100) / 100.0f);
            light.Intensity = 20.0f;
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

    virtual void OnUpdate(float deltaTime) override {
        for (auto& action : m_deferredActions) action();
        m_deferredActions.clear();

        auto* renderModule = FModuleManager::Get().GetModule<URenderModule>("Engine.Renderer");
        auto* windowMod = FModuleManager::Get().GetModule<UWindowModule>("Engine.Window");
        
        if (!renderModule || !windowMod) return;

        GLFWwindow* window = windowMod->GetNativeWindow();
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w <= 0 || h <= 0) return;

        // Resize renderer if needed
        renderModule->Resize(w, h);

        // Camera Control
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

        // Prepare Context
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
        
        // Scene Collection
        m_rootNode->UpdateWorldMatrix();
        m_deferredList.clear();
        m_forwardList.clear();
        m_pointLights.clear();
        CollectRenderables(m_rootNode.get());
        ctx.PointLights = m_pointLights;

        // Render!
        renderModule->Render(ctx, m_deferredList, m_forwardList);
    }

    void CollectRenderables(FSceneNode* node) {
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

    virtual void OnImGuiRender() override {
        ImGui::Begin("Scene Hierarchy");
        if (m_rootNode) DrawSceneNode(m_rootNode.get());
        ImGui::End();

        ImGui::Begin("Inspector");
        if (m_selectedNode) {
            ImGui::Text("Name: %s", m_selectedNode->GetName().c_str());
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                glm::vec3 pos = m_selectedNode->GetPosition();
                if (ImGui::DragFloat3("Position", &pos[0], 0.1f)) m_selectedNode->SetPosition(pos);
                glm::quat rot = m_selectedNode->GetRotation();
                glm::vec3 euler = glm::degrees(glm::eulerAngles(rot));
                if (ImGui::DragFloat3("Rotation", &euler[0], 0.1f)) m_selectedNode->SetRotation(glm::quat(glm::radians(euler)));
                glm::vec3 scale = m_selectedNode->GetScale();
                if (ImGui::DragFloat3("Scale", &scale[0], 0.1f)) m_selectedNode->SetScale(scale);
            }
            if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& renderables = m_selectedNode->GetRenderablesMutable();
                for (size_t i = 0; i < renderables.size(); ++i) {
                    auto mat = renderables[i].Material;
                    if (mat) {
                        ImGui::PushID((int)i);
                        if (ImGui::TreeNode(mat->GetName().c_str())) {
                            if (auto pbrMat = std::dynamic_pointer_cast<FStandardPBRMaterial>(mat)) {
                                float met = pbrMat->GetMetallic(); float roug = pbrMat->GetRoughness(); glm::vec3 alb = pbrMat->GetAlbedo();
                                if (ImGui::ColorEdit3("Albedo", &alb[0])) pbrMat->SetParameter("albedo", alb);
                                if (ImGui::SliderFloat("Metallic", &met, 0.0f, 1.0f)) pbrMat->SetParameter("metallic", met);
                                if (ImGui::SliderFloat("Roughness", &roug, 0.0f, 1.0f)) pbrMat->SetParameter("roughness", roug);
                            }
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                }
            }
        }
        ImGui::End();

        ImGui::Begin("Asset Loader");
        static char modelPath[256] = "";
        ImGui::InputText("Model Path", modelPath, 256);
        if (ImGui::Button("Browse...")) {
            auto selection = pfd::open_file("Select a 3D Model", ".", { "3D Models", "*.obj *.fbx *.gltf *.glb", "All Files", "*" }).result();
            if (!selection.empty()) strncpy(modelPath, selection[0].c_str(), 256);
        }
        ImGui::SameLine();
        if (ImGui::Button("Load Model")) {
            auto* renderModule = FModuleManager::Get().GetModule<URenderModule>("Engine.Renderer");
            if (renderModule) {
                if (auto newModel = FAssetLoader::LoadModel(*renderModule->GetDevice(), modelPath)) {
                    auto modelNode = std::make_unique<FSceneNode>("ImportedModel");
                    for (size_t i = 0; i < newModel->Renderables.size(); ++i) {
                        auto meshNode = std::make_unique<FSceneNode>("Mesh_" + std::to_string(i));
                        meshNode->AddRenderable(newModel->Renderables[i]);
                        modelNode->AddChild(std::move(meshNode));
                    }
                    if (m_selectedNode) m_selectedNode->AddChild(std::move(modelNode));
                    else m_rootNode->AddChild(std::move(modelNode));
                }
            }
        }
        if (ImGui::Button("Reset Camera")) { m_cameraPos = glm::vec3(0.0f, 0.0f, 10.0f); m_cameraYaw = -90.0f; m_cameraPitch = 0.0f; }
        ImGui::End();
    }

    void DrawSceneNode(FSceneNode* node) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (node == m_selectedNode) flags |= ImGuiTreeNodeFlags_Selected;
        if (node->GetChildren().empty()) flags |= ImGuiTreeNodeFlags_Leaf;
        bool isVis = node->IsVisible();
        ImGui::PushID(node);
        if (ImGui::Checkbox("##Visible", &isVis)) node->SetVisible(isVis);
        ImGui::PopID(); ImGui::SameLine();
        bool opened = ImGui::TreeNodeEx((void*)node, flags, node->GetName().c_str());
        if (ImGui::IsItemClicked()) m_selectedNode = node;
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                if (node->GetParent()) {
                    FSceneNode* n = node;
                    m_deferredActions.push_back([this, n]() { n->GetParent()->RemoveChild(n); });
                    if (m_selectedNode == node) m_selectedNode = nullptr;
                }
            }
            ImGui::EndPopup();
        }
        if (opened) {
            for (const auto& child : node->GetChildren()) DrawSceneNode(child.get());
            ImGui::TreePop();
        }
    }

private:
    std::shared_ptr<FStandardPBRMaterial> m_pbrMat;
    std::shared_ptr<IRHIBuffer> m_sphereVB, m_sphereIB;
    uint32_t m_sphereIndexCount = 0;
    
    // Scene
    std::unique_ptr<FSceneNode> m_rootNode;
    FSceneNode* m_selectedNode = nullptr;
    std::vector<FRenderable> m_deferredList;
    std::vector<FRenderable> m_forwardList;
    std::vector<FPointLight> m_pointLights;
    std::vector<std::function<void()>> m_deferredActions;

    glm::vec3 m_cameraPos{ 0.0f, 0.0f, 10.0f };
    float m_cameraYaw = -90.0f;
    float m_cameraPitch = 0.0f;
    bool m_firstMouse = true;
    double m_lastX = 0.0, m_lastY = 0.0;
};

AEngine::FApplication* AEngine::CreateApplication() { return new SandboxApp(); }

int main() {
    auto* app = AEngine::CreateApplication();
    app->Run();
    delete app;
    return 0;
}