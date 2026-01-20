#pragma once

#include <cstdint>

namespace AEngine {

    enum class ERHIPixelFormat {
        RGBA8_UNORM,
        RGBA16_FLOAT,
        D24_S8
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
