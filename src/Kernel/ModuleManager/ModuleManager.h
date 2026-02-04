#pragma once

#include "Kernel/Core/Module.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <nlohmann/json.hpp>

namespace AEngine {

    struct FModuleInfo {
        std::string Name;
        std::string FilePath;
        std::vector<std::string> Dependencies;
        EModuleLoadingPhase Phase = EModuleLoadingPhase::Default;
        bool bIsEnabled = false;
        bool bHasDll = true;

        static FModuleInfo FromJson(const nlohmann::json& j);
    };

    class FModuleManager {
    public:
        static AE_API FModuleManager& Get();

        using ModuleFactory = std::function<std::unique_ptr<IModule>()>;
        AE_API void RegisterStaticModule(const std::string& name, ModuleFactory factory);

        AE_API void DiscoverModules(const std::string& rootPath);
        AE_API void ResolveDependencies(const std::vector<std::string>& enabledModules);
        AE_API void StartupModules();
        AE_API void ShutdownModules();
        AE_API void UpdateModules(float deltaTime);

        template<typename T>
        T* GetModule(const std::string& name) {
            auto it = m_loadedModules.find(name);
            if (it != m_loadedModules.end()) {
                return dynamic_cast<T*>(it->second.get());
            }
            return nullptr;
        }

    private:
        FModuleManager() = default;
        ~FModuleManager() = default;

        void SortModulesByDependency();

        std::map<std::string, ModuleFactory> m_staticFactories;
        std::map<std::string, FModuleInfo> m_discoveredModules;
        std::vector<std::string> m_activeModuleNames;
        std::map<std::string, std::unique_ptr<IModule>> m_loadedModules;
        
        // 存储动态库句柄，key 为模块名
        std::map<std::string, void*> m_moduleHandles; 
    };

}
