#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace AEngine {

    /// @brief Standard Vertex layout used across the engine.
    /// Matched to Shader layout locations:
    /// 0: Position
    /// 1: Normal
    /// 2: TexCoords
    /// 3: Tangent
    /// 4: Bitangent
    /// 5: Color
    struct FVertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
        glm::vec4 Color;
    };

    /// @brief Supported Texture Formats.
    enum class ERHIPixelFormat {
        /// @brief Standard 8-bit per channel RGBA. Used for Albedo, Emissive.
        RGBA8_UNORM,
        /// @brief 16-bit float per channel RGBA. Used for HDR Color, Normals (precision), Positions.
        RGBA16_FLOAT,
        /// @brief Depth (24-bit) + Stencil (8-bit). Standard Depth Buffer.
        D24_S8,
        /// @brief Depth (24-bit) only. Used for Shadow Maps.
        Depth24 
    };

    /// @brief Buffer Usage Hint (for driver optimization).
    enum class ERHIBufferUsage {
        /// @brief Data set once, rarely changed. (GL_STATIC_DRAW)
        Static,
        /// @brief Data changed frequently. (GL_DYNAMIC_DRAW)
        Dynamic
    };

    enum class ERHIBufferType {
        Vertex,
        Index,
        Uniform
    };

    enum class ERHIShaderStage {
        Vertex,
        Fragment,
        Compute
    };

}
