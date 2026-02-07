#pragma once

#include "Engine.RHI/RenderGraph.h"
#include <array>

namespace AEngine {

    struct FFrustumPlane {
        glm::vec3 Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        float Distance = 0.0f;
    };

    class FFrustumCulling {
    public:
        static std::array<FFrustumPlane, 6> ExtractPlanes(const glm::mat4& viewProjection);
        static bool IsSphereVisible(const std::array<FFrustumPlane, 6>& planes, const glm::vec3& center, float radius);
        static std::vector<FRenderable> CullByCameraFrustum(const FRenderContext& context, const std::vector<FRenderable>& input);
    };

}

