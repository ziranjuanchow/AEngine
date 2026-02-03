#pragma once
#include "Kernel/Core/Module.h"
#include "IRHIDevice.h"
#include <memory>

namespace AEngine {
    class IRHIModule : public IModule {
    public:
        virtual std::shared_ptr<IRHIDevice> CreateDevice() = 0;
    };
}
