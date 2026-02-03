#pragma once

#include "Kernel/Core/EngineTypes.h"
#include "Plugin.h"
#include "Module.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <windows.h> 

namespace AEngine {

    enum class PluginError {
        FileNotFound,
        LoadFailed,
        EntryPointNotFound,
        AlreadyLoaded
    };

    class APluginManager {
    public:
        static AE_API APluginManager& Get();

        AE_API AEngine::expected<IPlugin*, PluginError> LoadPlugin(const std::string& path);
        AE_API void UnloadPlugin(const std::string& name);
        AE_API void UnloadAll();

        AE_API IPlugin* GetPlugin(const std::string& name) const;

    private:
        APluginManager() = default;
        ~APluginManager();

        struct PluginData {
            HMODULE handle;
            std::unique_ptr<IPlugin> instance;
        };

        std::unordered_map<std::string, PluginData> m_loadedPlugins;
    };

}