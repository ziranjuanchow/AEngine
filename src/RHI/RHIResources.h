#pragma once

#include <memory>
#include <vector>
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

    class IRHIFramebuffer : public IRHIResource {
    public:
        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual std::shared_ptr<IRHITexture> GetDepthAttachment() const = 0;
    };

    class IRHIPipelineState : public IRHIResource {};

}