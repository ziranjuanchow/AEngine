#include "PluginManager.h"
#include "Log.h"
#include <imgui.h>

namespace AEngine {

    APluginManager& APluginManager::Get() {
        static APluginManager instance;
        return instance;
    }

    APluginManager::~APluginManager() {
        UnloadAll();
    }

    AEngine::expected<IPlugin*, PluginError> APluginManager::LoadPlugin(const std::string& path) {
        AE_CORE_INFO("Attempting to load plugin: {0}", path);

        HMODULE handle = LoadLibraryA(path.c_str());
        if (!handle) {
            AE_CORE_ERROR("Failed to load library: {0}", path);
            return AEngine::unexpected(PluginError::FileNotFound);
        }

        auto createFunc = (CreatePluginFunc)GetProcAddress(handle, "CreatePlugin");
        if (!createFunc) {
            AE_CORE_ERROR("Failed to find CreatePlugin entry point in: {0}", path);
            FreeLibrary(handle);
            return AEngine::unexpected(PluginError::EntryPointNotFound);
        }

        IPlugin* instance = createFunc();
        if (!instance) {
            AE_CORE_ERROR("CreatePlugin returned nullptr in: {0}", path);
            FreeLibrary(handle);
            return AEngine::unexpected(PluginError::LoadFailed);
        }

        std::string name = instance->GetName();
        if (m_loadedPlugins.contains(name)) {
            AE_CORE_WARN("Plugin {0} is already loaded.", name);
            delete instance;
            FreeLibrary(handle);
            return AEngine::unexpected(PluginError::AlreadyLoaded);
        }

        instance->Initialize(ImGui::GetCurrentContext());
        instance->OnLoad();
        AE_CORE_INFO("Successfully loaded plugin: {0}", name);

        m_loadedPlugins[name] = { handle, std::unique_ptr<IPlugin>(instance) };
        return instance;
    }

    void APluginManager::UnloadPlugin(const std::string& name) {
        auto it = m_loadedPlugins.find(name);
        if (it != m_loadedPlugins.end()) {
            AE_CORE_INFO("Unloading plugin: {0}", name);
            it->second.instance->OnUnload();
            it->second.instance.reset(); // Destroys instance
            FreeLibrary(it->second.handle);
            m_loadedPlugins.erase(it);
        }
    }

    void APluginManager::UnloadAll() {
        AE_CORE_INFO("Unloading all plugins...");
        for (auto& [name, data] : m_loadedPlugins) {
            data.instance->OnUnload();
            data.instance.reset();
            FreeLibrary(data.handle);
        }
        m_loadedPlugins.clear();
    }

    IPlugin* APluginManager::GetPlugin(const std::string& name) const {
        auto it = m_loadedPlugins.find(name);
        return it != m_loadedPlugins.end() ? it->second.instance.get() : nullptr;
    }

}
