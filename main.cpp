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
        // if (auto loadResult = pluginManager.LoadPlugin("StatsPanel.dll"); !loadResult) {
        //     AE_CORE_ERROR("Failed to load StatsPanel plugin!");
        // }

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
        m_shadowPass = shadowPass.get(); // Keep raw ptr
        
        m_renderGraph->AddPass(std::move(shadowPass));
        m_renderGraph->AddPass(std::make_unique<FForwardLitPass>());

        // Material Setup
        m_pbrMat = std::make_shared<FStandardPBRMaterial>("StandardPBR");
        m_pbrMat->LoadShaders("shaders/StandardPBR.vert", "shaders/StandardPBR.frag");
        m_pbrMat->SetParameter("albedo", glm::vec3(0.5f, 0.0f, 0.0f)); // Red
        m_pbrMat->SetParameter("ao", 1.0f);

        // Geometry Setup (Fallback)
        uint32_t sphereIndexCount;
        FGeometryUtils::CreateSphere(*m_device, m_sphereVB, m_sphereIB, sphereIndexCount);

        std::shared_ptr<IRHIBuffer> quadVB, quadIB;
        uint32_t quadIndexCount;
        FGeometryUtils::CreateQuad(*m_device, quadVB, quadIB, quadIndexCount);

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
            r.WorldMatrix = glm::mat4(1.0f);
            groundNode->AddRenderable(r);
        }
        m_rootNode->AddChild(std::move(groundNode));

        for (int i = 0; i < 5; ++i) {
            auto sphereNode = std::make_unique<FSceneNode>("Sphere_" + std::to_string(i));
            sphereNode->SetPosition(glm::vec3(i * 2.5f - 5.0f, 0.0f, 0.0f));
            
            FRenderable r;
            r.VertexBuffer = m_sphereVB;
            r.IndexBuffer = m_sphereIB;
            r.IndexCount = sphereIndexCount;
            r.Material = m_pbrMat;
            // WorldMatrix will be overwritten by Node's transform during collection
            
            sphereNode->AddRenderable(r);
            m_rootNode->AddChild(std::move(sphereNode));
        }

        m_cmdBuffer = m_device->CreateCommandBuffer();
    }

    virtual void OnUpdate(float deltaTime) override {
        // Camera Control
        GLFWwindow* window = m_window->GetNativeWindow();
        
        // Movement
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
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) m_cameraPos += glm::vec3(0.0f, 1.0f, 0.0f) * speed;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) m_cameraPos -= glm::vec3(0.0f, 1.0f, 0.0f) * speed;

        // Rotation (Right Mouse Button)
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            if (m_firstMouse) {
                m_lastX = xpos;
                m_lastY = ypos;
                m_firstMouse = false;
            }

            float xoffset = (float)(xpos - m_lastX);
            float yoffset = (float)(m_lastY - ypos); // Reversed since y-coordinates range from bottom to top
            m_lastX = xpos;
            m_lastY = ypos;

            float sensitivity = 0.05f; // Reduced from 0.1f
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            m_cameraYaw   += xoffset;
            m_cameraPitch += yoffset;

            if(m_cameraPitch > 89.0f)  m_cameraPitch = 89.0f;
            if(m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            m_firstMouse = true;
        }

        // Clear Screen
        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        
        if (w <= 0 || h <= 0) return; // Window minimized or invalid size

        glViewport(0, 0, w, h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        // Prepare Render Context
        FRenderContext ctx;
        ctx.ViewMatrix = glm::lookAt(m_cameraPos, m_cameraPos + cameraFront, cameraUp);
        ctx.ProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100.0f);
        ctx.CameraPosition = m_cameraPos;
        ctx.LightPosition = glm::vec3(10.0f, 10.0f, 10.0f);
        ctx.LightColor = glm::vec3(150.0f, 150.0f, 150.0f);

        // Shadow Matrix
        float near_plane = 1.0f, far_plane = 50.0f;
        glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(ctx.LightPosition, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        ctx.LightSpaceMatrix = lightProjection * lightView;

        // Update Scene
        m_rootNode->UpdateWorldMatrix();
        m_scene.clear();
        CollectRenderables(m_rootNode.get(), m_scene);

        // Bind Shadow Map to PBR Material
        // In a real engine, this would be handled by the Render Graph automatically
        m_pbrMat->SetParameter("shadowMap", m_shadowPass->GetDepthMap());
        m_pbrMat->SetParameter("lightSpaceMatrix", ctx.LightSpaceMatrix); // Forward Pass needs this too? Yes via context

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
                // ...
            }
        }
        
        if (ImGui::Button("Reset Camera")) {
            m_cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
            m_cameraYaw = -90.0f;
            m_cameraPitch = 0.0f;
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

        ImGui::Begin("Shadow Settings");
        static float biasConstant = 1.1f;
        static float biasSlope = 4.0f;
        ImGui::SliderFloat("Bias Constant", &biasConstant, 0.0f, 10.0f);
        ImGui::SliderFloat("Bias Slope", &biasSlope, 0.0f, 10.0f);
        // Note: Currently these are applied in ShadowPass.cpp via hardcode or we can pass via Context
        // To simplify, let's just use them in the next frame.
        // TODO: Pass these to m_shadowPass
        ImGui::End();
    }

private:
    std::shared_ptr<FOpenGLDevice> m_device;
    std::unique_ptr<FRenderGraph> m_renderGraph;
    FShadowPass* m_shadowPass = nullptr; // Raw ptr
    std::shared_ptr<IRHICommandBuffer> m_cmdBuffer;
    
    std::shared_ptr<FStandardPBRMaterial> m_pbrMat;
    std::shared_ptr<IRHIBuffer> m_sphereVB, m_sphereIB;
    
    // Scene
    std::unique_ptr<FSceneNode> m_rootNode;
    std::vector<FRenderable> m_scene; // Flattened list for rendering
    std::shared_ptr<FModel> m_currentModel;

    void CollectRenderables(FSceneNode* node, std::vector<FRenderable>& outList) {
        if (!node->IsVisible()) return;

        // Add node's renderables
        for (auto renderable : node->GetRenderables()) {
            // Update World Matrix from Node
            renderable.WorldMatrix = node->GetWorldMatrix();
            outList.push_back(renderable);
        }

        // Recurse
        for (const auto& child : node->GetChildren()) {
            CollectRenderables(child.get(), outList);
        }
    }

    // Camera
    glm::vec3 m_cameraPos{ 0.0f, 0.0f, 10.0f };
    float m_cameraYaw = -90.0f;
    float m_cameraPitch = 0.0f;
    bool m_firstMouse = true;
    double m_lastX = 0.0;
    double m_lastY = 0.0;
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