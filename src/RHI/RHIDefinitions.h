#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace AEngine {

    struct FVertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
        glm::vec4 Color;
    };

    enum class ERHIPixelFormat {
        RGBA8_UNORM,
        RGBA16_FLOAT,
        D24_S8,
        Depth24 // Depth component only
    };

    enum class ERHIBufferUsage {
        Static,
        Dynamic
    };

    enum class ERHIBufferType {
        Vertex,
        Index,
        Uniform
    };

}
