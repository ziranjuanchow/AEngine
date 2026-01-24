#include <iostream>
#include <glad/glad.h>
#include "Core/FApplication.h"
#include "Core/Log.h"
#include "Core/SceneNode.h"
#include "Core/PluginManager.h"
#include "Core/GeometryUtils.h"
#include "Core/AssetLoader.h"
#include "RHI/OpenGL/OpenGLDevice.h"
#include "RHI/ForwardLitPass.h"
#include "RHI/ShadowPass.h"
#include "RHI/DeferredGeometryPass.h"
#include "RHI/DeferredLightingPass.h"
#include "RHI/StandardPBRMaterial.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <portable-file-dialogs.h>
#include <functional>

using namespace AEngine;

class SandboxApp : public FApplication {
public:
    SandboxApp() : FApplication({ "AEngine Sandbox", 1600, 900 }) {
        // NFD is handled by pfd (portable-file-dialogs) which doesn't need explicit Init in simple cases
    }

    virtual void OnInit() override {
        AE_CORE_INFO("SandboxApp Init");

        // RHI & Pipeline Setup
        m_device = std::make_shared<FOpenGLDevice>();
        m_renderGraph = std::make_unique<FRenderGraph>();
        
        // Shadow Pass Setup
        FFramebufferConfig shadowConfig;
        shadowConfig.Width = 2048;
        shadowConfig.Height = 2048;
        shadowConfig.DepthAttachment = m_device->CreateTexture(2048, 2048, ERHIPixelFormat::Depth24);
        auto shadowFBO = m_device->CreateFramebuffer(shadowConfig);
        
        auto shadowPass = std::make_unique<FShadowPass>(shadowFBO);
        m_shadowPass = shadowPass.get(); 
        m_renderGraph->AddPass(std::move(shadowPass));

        // Deferred Pass Setup
        FFramebufferConfig gBufferConfig;
        gBufferConfig.Width = 1600; 
        gBufferConfig.Height = 900;
        gBufferConfig.DepthAttachment = m_device->CreateTexture(1600, 900, ERHIPixelFormat::D24_S8);
        gBufferConfig.ColorAttachments.push_back(m_device->CreateTexture(1600, 900, ERHIPixelFormat::RGBA8_UNORM)); // Albedo
        gBufferConfig.ColorAttachments.push_back(m_device->CreateTexture(1600, 900, ERHIPixelFormat::RGBA16_FLOAT)); // Normal
        gBufferConfig.ColorAttachments.push_back(m_device->CreateTexture(1600, 900, ERHIPixelFormat::RGBA8_UNORM)); // Emissive
        
        auto gBuffer = m_device->CreateFramebuffer(gBufferConfig);
        m_renderGraph->AddPass(std::make_unique<FDeferredGeometryPass>(gBuffer));

        // Geometry Setup (Fallback)
        uint32_t sphereIndexCount;
        FGeometryUtils::CreateSphere(*m_device, m_sphereVB, m_sphereIB, sphereIndexCount);
        m_sphereIndexCount = sphereIndexCount;

        std::shared_ptr<IRHIBuffer> quadVB, quadIB;
        uint32_t quadIndexCount;
        FGeometryUtils::CreateQuad(*m_device, quadVB, quadIB, quadIndexCount);

        // Lighting Pass needs sphere mesh
        m_renderGraph->AddPass(std::make_unique<FDeferredLightingPass>(gBuffer, m_sphereVB, m_sphereIB, m_sphereIndexCount));

        m_renderGraph->AddPass(std::make_unique<FForwardLitPass>()); 

        // Material Setup
        m_pbrMat = std::make_shared<FStandardPBRMaterial>("StandardPBR");
        m_pbrMat->LoadShaders("shaders/StandardPBR.vert", "shaders/StandardPBR.frag");
        m_pbrMat->SetParameter("albedo", glm::vec3(0.5f, 0.0f, 0.0f)); // Red
        m_pbrMat->SetParameter("ao", 1.0f);

        // Initial Scene Setup
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

        m_cmdBuffer = m_device->CreateCommandBuffer();

        // Point Lights Setup
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
            
            // Optional: Add a visual mesh for the light (Debug)
            FRenderable debugSphere;
            debugSphere.VertexBuffer = m_sphereVB;
            debugSphere.IndexBuffer = m_sphereIB;
            debugSphere.IndexCount = m_sphereIndexCount;
            debugSphere.Material = m_pbrMat; // Use PBR mat for now, just to see it
            debugSphere.WorldMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f)); 
            lightNode->AddRenderable(debugSphere);

            lightRoot->AddChild(std::move(lightNode));
        }
        m_rootNode->AddChild(std::move(lightRoot));
    }

    virtual void OnUpdate(float deltaTime) override {
        for (auto& action : m_deferredActions) action();
        m_deferredActions.clear();

        GLFWwindow* window = m_window->GetNativeWindow();
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

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        if (w <= 0 || h <= 0) return;

        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

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
        // Update Scene
        m_rootNode->UpdateWorldMatrix();
        m_scene.clear();
        m_pointLights.clear();
        CollectRenderables(m_rootNode.get(), m_scene, m_pointLights);
        ctx.PointLights = m_pointLights;

        for (auto& r : m_scene) {
            if (auto mat = std::dynamic_pointer_cast<FStandardPBRMaterial>(r.Material)) {
                mat->SetParameter("shadowMap", m_shadowPass->GetDepthMap());
            }
        }

        // Execute Render Graph
        m_cmdBuffer->Begin();
        
        // Manual Pass Order Control
        m_renderGraph->Execute(*m_cmdBuffer, ctx, m_scene);
        
        // Final Sync: Blit depth from G-Buffer to screen so Forward pass can use it
        m_device->BlitFramebuffer(m_gBuffer, w, h);
        
        m_cmdBuffer->End();

    }

    void CollectRenderables(FSceneNode* node, std::vector<FRenderable>& outList, std::vector<FPointLight>& outLights) {
        if (!node->IsVisible()) return;

        // Add node's renderables
        for (auto renderable : node->GetRenderables()) {
            renderable.WorldMatrix = node->GetWorldMatrix();
            outList.push_back(renderable);
        }

        // Add node's light (Sync position with node)
        if (node->HasPointLight()) {
            FPointLight light = node->GetPointLight().value();
            light.Position = glm::vec3(node->GetWorldMatrix()[3]);
            outLights.push_back(light);
        }

        // Recurse
        for (const auto& child : node->GetChildren()) {
            CollectRenderables(child.get(), outList, outLights);
        }
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
            if (auto newModel = FAssetLoader::LoadModel(*m_device, modelPath)) {
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
    std::shared_ptr<FOpenGLDevice> m_device;
    std::unique_ptr<FRenderGraph> m_renderGraph;
    FShadowPass* m_shadowPass = nullptr;
    std::shared_ptr<IRHIFramebuffer> m_gBuffer;
    std::shared_ptr<IRHICommandBuffer> m_cmdBuffer;
    std::shared_ptr<FStandardPBRMaterial> m_pbrMat;
    std::shared_ptr<IRHIBuffer> m_sphereVB, m_sphereIB;
    uint32_t m_sphereIndexCount = 0;
    std::unique_ptr<FSceneNode> m_rootNode;
    FSceneNode* m_selectedNode = nullptr;
    std::vector<FRenderable> m_scene;
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
