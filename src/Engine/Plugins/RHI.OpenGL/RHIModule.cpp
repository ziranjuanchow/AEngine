#include "Engine.RHI/RHIModule.h"
#include "OpenGLDevice.h"

namespace AEngine {
    class FOpenGLModule : public IRHIModule {
    public:
        virtual void OnStartup() override {}
        virtual void OnShutdown() override {}
        
        virtual std::shared_ptr<IRHIDevice> CreateDevice() override {
            return std::make_shared<FOpenGLDevice>();
        }
    };
}

AE_IMPLEMENT_MODULE(AEngine::FOpenGLModule)
