#include "FrustumCulling.h"
#include <algorithm>
#include <cmath>

namespace AEngine {

    static FFrustumPlane NormalizePlane(const glm::vec4& plane) {
        const glm::vec3 normal(plane.x, plane.y, plane.z);
        const float len = glm::length(normal);
        if (len <= 1e-6f) {
            return {};
        }
        return { normal / len, plane.w / len };
    }

    static float MaxAxisScale(const glm::mat4& world) {
        const float sx = glm::length(glm::vec3(world[0]));
        const float sy = glm::length(glm::vec3(world[1]));
        const float sz = glm::length(glm::vec3(world[2]));
        return std::max(sx, std::max(sy, sz));
    }

    std::array<FFrustumPlane, 6> FFrustumCulling::ExtractPlanes(const glm::mat4& viewProjection) {
        const glm::mat4 m = glm::transpose(viewProjection);
        std::array<FFrustumPlane, 6> planes = {
            NormalizePlane(m[3] + m[0]), // Left
            NormalizePlane(m[3] - m[0]), // Right
            NormalizePlane(m[3] + m[1]), // Bottom
            NormalizePlane(m[3] - m[1]), // Top
            NormalizePlane(m[3] + m[2]), // Near
            NormalizePlane(m[3] - m[2])  // Far
        };
        return planes;
    }

    bool FFrustumCulling::IsSphereVisible(const std::array<FFrustumPlane, 6>& planes, const glm::vec3& center, float radius) {
        for (const auto& plane : planes) {
            const float signedDistance = glm::dot(plane.Normal, center) + plane.Distance;
            if (signedDistance < -radius) {
                return false;
            }
        }
        return true;
    }

    std::vector<FRenderable> FFrustumCulling::CullByCameraFrustum(const FRenderContext& context, const std::vector<FRenderable>& input) {
        if (!context.EnableFrustumCulling || input.empty()) {
            return input;
        }

        const auto planes = ExtractPlanes(context.ProjectionMatrix * context.ViewMatrix);
        std::vector<FRenderable> visible;
        visible.reserve(input.size());

        for (const auto& renderable : input) {
            const glm::vec3 center(renderable.WorldMatrix[3]);
            const float worldRadius = std::max(0.001f, renderable.BoundingRadius * MaxAxisScale(renderable.WorldMatrix));
            if (IsSphereVisible(planes, center, worldRadius)) {
                visible.push_back(renderable);
            }
        }

        return visible;
    }

}

