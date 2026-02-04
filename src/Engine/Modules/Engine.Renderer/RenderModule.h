#pragma once
#include "Kernel/Core/Module.h"
#include "Engine.RHI/IRHIDevice.h"
#include "SceneRenderer.h"
#include <memory>
#include <vector>

namespace AEngine {
    class URenderModule : public IModule {
    public:
        virtual void OnStartup() override;
        virtual void OnShutdown() override;

        void Render(const FRenderContext& ctx, 
                    const std::vector<FRenderable>& deferred, 
                    const std::vector<FRenderable>& forward);
        
        void Resize(uint32_t width, uint32_t height);
        
        std::shared_ptr<IRHIDevice> GetDevice() const { return m_device; }

    private:
        bool TryInitDevice();

        std::shared_ptr<IRHIDevice> m_device;
        std::unique_ptr<FSceneRenderer> m_renderer;
    };
}
