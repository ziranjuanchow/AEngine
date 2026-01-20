#pragma once

#include <memory>
#include "RHIDefinitions.h"

namespace AEngine {

    class IRHIResource {
    public:
        virtual ~IRHIResource() = default;
    };

    class IRHIBuffer : public IRHIResource {
    public:
        virtual uint32_t GetSize() const = 0;
    };

    class IRHITexture : public IRHIResource {
    public:
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
    };

    class IRHIPipelineState : public IRHIResource {};

}
