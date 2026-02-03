#include "ModuleManager.h"
#include "Kernel/Core/Log.h"
#include <fstream>
#include <filesystem>
#include <stack>

namespace AEngine {

    namespace fs = std::filesystem;

    FModuleInfo FModuleInfo::FromJson(const nlohmann::json& j) {
        FModuleInfo info;
        info.Name = j.value("name", "Unknown");
        if (j.contains("dependencies")) {
            info.Dependencies = j["dependencies"].get<std::vector<std::string>>();
        }
        std::string phaseStr = j.value("phase", "Default");
        if (phaseStr == "Earliest") info.Phase = EModuleLoadingPhase::Earliest;
        else if (phaseStr == "PreDefault") info.Phase = EModuleLoadingPhase::PreDefault;
        else if (phaseStr == "PostDefault") info.Phase = EModuleLoadingPhase::PostDefault;
        else if (phaseStr == "Latest") info.Phase = EModuleLoadingPhase::Latest;
        else info.Phase = EModuleLoadingPhase::Default;
        
        return info;
    }

    FModuleManager& FModuleManager::Get() {
        static FModuleManager instance;
        return instance;
    }

    void FModuleManager::DiscoverModules(const std::string& rootPath) {
        AE_CORE_INFO("Scanning for modules in: {0}", rootPath);
        
        for (const auto& entry : fs::recursive_directory_iterator(rootPath)) {
            if (entry.is_regular_file() && entry.path().filename() == "module.json") {
                try {
                    std::ifstream f(entry.path());
                    nlohmann::json j = nlohmann::json::parse(f);
                    FModuleInfo info = FModuleInfo::FromJson(j);
                    info.FilePath = entry.path().parent_path().string();
                    
                    AE_CORE_TRACE("Found module: {0} at {1}", info.Name, info.FilePath);
                    m_discoveredModules[info.Name] = info;
                } catch (const std::exception& e) {
                    AE_CORE_ERROR("Failed to parse module.json at {0}: {1}", entry.path().string(), e.what());
                }
            }
        }
    }

    void FModuleManager::ResolveDependencies(const std::vector<std::string>& enabledModules) {
        AE_CORE_INFO("Resolving dependencies...");
        
        m_activeModuleNames.clear();
        std::set<std::string> visited;
        std::set<std::string> visiting;

        std::function<void(const std::string&)> visit = [&](const std::string& name) {
            if (visited.count(name)) return;
            if (visiting.count(name)) {
                AE_CORE_CRITICAL("Circular dependency detected involving module: {0}", name);
                return;
            }

            visiting.insert(name);

            auto it = m_discoveredModules.find(name);
            if (it == m_discoveredModules.end()) {
                AE_CORE_ERROR("Module not found: {0}", name);
                return;
            }

            for (const auto& dep : it->second.Dependencies) {
                visit(dep);
            }

            visiting.erase(name);
            visited.insert(name);
            m_activeModuleNames.push_back(name);
        };

        for (const auto& name : enabledModules) {
            visit(name);
        }

        AE_CORE_INFO("Dependency resolution complete. Active modules: {0}", m_activeModuleNames.size());
    }

    void FModuleManager::RegisterStaticModule(const std::string& name, ModuleFactory factory) {
        m_staticFactories[name] = std::move(factory);
        
        // 如果该模块尚未被发现（没有 JSON），则创建一个基础信息
        if (m_discoveredModules.find(name) == m_discoveredModules.end()) {
            FModuleInfo info;
            info.Name = name;
            info.bIsEnabled = true;
            m_discoveredModules[name] = info;
        }
    }

    void FModuleManager::StartupModules() {
        AE_CORE_INFO("Starting up modules...");
        
        for (const auto& name : m_activeModuleNames) {
            auto factoryIt = m_staticFactories.find(name);
            if (factoryIt != m_staticFactories.end()) {
                AE_CORE_TRACE("Instantiating static module: {0}", name);
                auto module = factoryIt->second();
                module->OnStartup();
                m_loadedModules[name] = std::move(module);
            } else {
                AE_CORE_ERROR("No factory registered for module: {0}", name);
            }
        }
    }

    void FModuleManager::ShutdownModules() {
        AE_CORE_INFO("Shutting down modules...");
        for (auto it = m_activeModuleNames.rbegin(); it != m_activeModuleNames.rend(); ++it) {
            auto loadedIt = m_loadedModules.find(*it);
            if (loadedIt != m_loadedModules.end()) {
                AE_CORE_TRACE("Shutting down module: {0}", *it);
                loadedIt->second->OnShutdown();
            }
        }
        m_loadedModules.clear();
    }

    void FModuleManager::UpdateModules(float deltaTime) {
        for (const auto& name : m_activeModuleNames) {
            auto it = m_loadedModules.find(name);
            if (it != m_loadedModules.end()) {
                it->second->OnUpdate(deltaTime);
            }
        }
    }
}
