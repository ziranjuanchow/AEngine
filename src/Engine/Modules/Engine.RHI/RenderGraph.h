#pragma once

#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "IRHIDevice.h"
#include "IMaterial.h"

namespace AEngine {

    struct FRenderable {
        std::shared_ptr<IRHIBuffer> VertexBuffer;
        std::shared_ptr<IRHIBuffer> IndexBuffer;
        std::shared_ptr<IMaterial> Material;
        glm::mat4 WorldMatrix;
        uint32_t IndexCount;
        // Coarse bound used by camera frustum culling (local-space sphere radius).
        float BoundingRadius = 1.0f;
    };

    struct FPointLight {
        glm::vec3 Position;
        glm::vec3 Color;
        float Intensity;
        float Radius;
    };

    struct FRenderContext {
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
        glm::vec3 CameraPosition;
        
        // Lighting
        glm::vec3 LightPosition; // Directional light position/direction
        glm::vec3 LightColor;
        glm::mat4 LightSpaceMatrix;

        std::vector<FPointLight> PointLights;
        bool EnableFrustumCulling = true;
    };

    class FRenderPass {
    public:
        virtual ~FRenderPass() = default;
        virtual void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) = 0;
        virtual std::string GetName() const = 0;
    };

    class FRenderGraph {
    public:
        void AddPass(std::unique_ptr<FRenderPass> pass) { m_passes.push_back(std::move(pass)); }
        void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables);
        
        // Exposed for manual control
        std::vector<FRenderPass*> GetPasses() {
            std::vector<FRenderPass*> rawPasses;
            for(auto& p : m_passes) rawPasses.push_back(p.get());
            return rawPasses;
        }

    private:
        std::vector<std::unique_ptr<FRenderPass>> m_passes;
    };

}
