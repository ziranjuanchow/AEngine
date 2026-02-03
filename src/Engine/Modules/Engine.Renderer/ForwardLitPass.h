#pragma once

#include "RenderGraph.h"

namespace AEngine {

    class FForwardLitPass : public FRenderPass {
    public:
        virtual void Execute(IRHICommandBuffer& cmdBuffer, const FRenderContext& context, const std::vector<FRenderable>& renderables) override;
        virtual std::string GetName() const override { return "ForwardLitPass"; }
    };

}
