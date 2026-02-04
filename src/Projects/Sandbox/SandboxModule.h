#pragma once

#include "Kernel/Core/Module.h"
#include "Engine.Scene/SceneNode.h"
#include "Engine.Renderer/StandardPBRMaterial.h"
#include "Engine.RHI/IRHIDevice.h"
#include "Engine.RHI/RHIResources.h"
#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

namespace AEngine {

    class SandboxModule : public IModule {
    public:
        virtual void OnStartup() override;
        virtual void OnShutdown() override;
        virtual void OnUpdate(float deltaTime) override;

    private:
        void CollectRenderables(FSceneNode* node);
        void DrawSceneNode(FSceneNode* node);
        void SetupScene();

        std::shared_ptr<IRHIDevice> m_device;
        
        std::shared_ptr<FStandardPBRMaterial> m_pbrMat;
        std::shared_ptr<IRHIBuffer> m_sphereVB, m_sphereIB;
        uint32_t m_sphereIndexCount = 0;
        
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

}
