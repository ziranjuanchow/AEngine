#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <windows.h> // For HMODULE
#include "Engine.h"
#include "Plugin.h"

namespace AEngine {

    enum class PluginError {
        FileNotFound,
        LoadFailed,
        EntryPointNotFound,
        AlreadyLoaded
    };

    class APluginManager {
    public:
        static APluginManager& Get();

        AEngine::expected<IPlugin*, PluginError> LoadPlugin(const std::string& path);
        void UnloadPlugin(const std::string& name);
        void UnloadAll();

        IPlugin* GetPlugin(const std::string& name) const;

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
