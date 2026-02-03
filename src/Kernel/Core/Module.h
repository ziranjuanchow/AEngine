#pragma once

#include <string>
#include <vector>

namespace AEngine {

    enum class EModuleLoadingPhase {
        Earliest,
        PreDefault,
        Default,
        PostDefault,
        Latest
    };

    class IModule {
    public:
        virtual ~IModule() = default;

        virtual void OnLoad() {}
        virtual void OnStartup() = 0;
        virtual void OnShutdown() = 0;
        virtual void OnUnload() {}
        virtual void OnUpdate(float deltaTime) {}
    };

}

#if defined(_MSC_VER)
    #define AE_DLL_EXPORT __declspec(dllexport)
    #define AE_DLL_IMPORT __declspec(dllimport)
#else
    #define AE_DLL_EXPORT __attribute__((visibility("default")))
    #define AE_DLL_IMPORT
#endif