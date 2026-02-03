#pragma once

#include <string>
#include <vector>

// 模块导出宏定义
#if defined(_MSC_VER)
    #define AE_EXPORT __declspec(dllexport)
    #define AE_IMPORT __declspec(dllimport)
#else
    #define AE_EXPORT __attribute__((visibility("default")))
    #define AE_IMPORT
#endif

#ifdef AENGINE_KERNEL_EXPORTS
    #define AE_API AE_EXPORT
#else
    #define AE_API AE_IMPORT
#endif

namespace AEngine {

    enum class EModuleLoadingPhase {
        Earliest,
        PreDefault,
        Default,
        PostDefault,
        Latest
    };

    class AE_API IModule {
    public:
        virtual ~IModule() = default;

        virtual void OnLoad() {}
        virtual void OnStartup() = 0;
        virtual void OnShutdown() = 0;
        virtual void OnUnload() {}
        virtual void OnUpdate(float deltaTime) {}
    };

}

// 模块工厂函数原型
typedef AEngine::IModule* (*CreateModuleFunc)();

// 模块实现辅助宏
#define AE_IMPLEMENT_MODULE(ModuleClass) \
    extern "C" AE_EXPORT AEngine::IModule* CreateModule() { \
        return new ModuleClass(); \
    }
