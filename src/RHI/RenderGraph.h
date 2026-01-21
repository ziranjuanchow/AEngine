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
    };

    struct FRenderContext {
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
        glm::vec3 CameraPosition;
        
        // Lighting
        glm::vec3 LightPosition;
        glm::vec3 LightColor;
        glm::mat4 LightSpaceMatrix;
    };

    class FRenderPass {
    public:
        virtual ~FRenderPass() = default;
        virtual void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) = 0;
        virtual std::string GetName() const = 0;
    };

    class FRenderGraph {
    public:
        void AddPass(std::unique_ptr<FRenderPass> pass) {
            m_passes.push_back(std::move(pass));
        }

        void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) {
            for (auto& pass : m_passes) {
                pass->Execute(cmdBuffer, context, renderables);
            }
        }

    private:
        std::vector<std::unique_ptr<FRenderPass>> m_passes;
    };

}
