#pragma once

#include <string>
#include <memory>

namespace AEngine {

    class IPlugin {
    public:
        virtual ~IPlugin() = default;

        virtual void Initialize(void* context) = 0;
        virtual void OnLoad() = 0;
        virtual void OnUnload() = 0;
        virtual void OnImGuiRender() = 0;
        virtual std::string GetName() const = 0;
    };

    // Plugin entry point signature
    typedef IPlugin* (*CreatePluginFunc)();

}
