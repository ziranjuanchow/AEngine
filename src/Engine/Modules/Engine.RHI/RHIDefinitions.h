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

    /// @brief Blending factors for RHI.
    enum class ERHIBlendFactor {
        Zero,                           // GL_ZERO
        One,                            // GL_ONE
        SrcColor,                       // GL_SRC_COLOR
        OneMinusSrcColor,               // GL_ONE_MINUS_SRC_COLOR
        DstColor,                       // GL_DST_COLOR
        OneMinusDstColor,               // GL_ONE_MINUS_DST_COLOR
        SrcAlpha,                       // GL_SRC_ALPHA
        OneMinusSrcAlpha,               // GL_ONE_MINUS_SRC_ALPHA
        DstAlpha,                       // GL_DST_ALPHA
        OneMinusDstAlpha,               // GL_ONE_MINUS_DST_ALPHA
        ConstantColor,                  // GL_CONSTANT_COLOR
        OneMinusConstantColor,          // GL_ONE_MINUS_CONSTANT_COLOR
        ConstantAlpha,                  // GL_CONSTANT_ALPHA
        OneMinusConstantAlpha,          // GL_ONE_MINUS_CONSTANT_ALPHA
        SrcAlphaSaturate                // GL_SRC_ALPHA_SATURATE
    };

    /// @brief Mask for specifying which buffers to blit (color, depth, stencil).
    enum class ERHIBlitMask {
        ColorBuffer = 0x00004000, // GL_COLOR_BUFFER_BIT
        DepthBuffer = 0x00000100, // GL_DEPTH_BUFFER_BIT
        StencilBuffer = 0x00000400, // GL_STENCIL_BUFFER_BIT
        All = ColorBuffer | DepthBuffer | StencilBuffer
    };

    /// @brief Filter for blitting (nearest, linear).
    enum class ERHIBlitFilter {
        Nearest = 0x2600, // GL_NEAREST
        Linear = 0x2601   // GL_LINEAR
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
