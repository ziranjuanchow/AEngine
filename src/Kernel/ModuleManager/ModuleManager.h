#pragma once

#include "Kernel/Core/Module.h"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <nlohmann/json.hpp>

namespace AEngine {

    /**
     * @brief 模块元数据
     */
    struct FModuleInfo {
        std::string Name;
        std::string FilePath;
        std::vector<std::string> Dependencies;
        EModuleLoadingPhase Phase = EModuleLoadingPhase::Default;
        bool bIsEnabled = false;

        static FModuleInfo FromJson(const nlohmann::json& j);
    };

    /**
     * @brief 模块管理器（微内核核心）
     */
    class FModuleManager {
    public:
        static FModuleManager& Get();

        /**
         * @brief 静态注册模块工厂（用于 Phase 0）
         */
        using ModuleFactory = std::function<std::unique_ptr<IModule>()>;
        void RegisterStaticModule(const std::string& name, ModuleFactory factory);

        /**
         * @brief 扫描指定目录下的模块描述文件
         */
        void DiscoverModules(const std::string& rootPath);

        /**
         * @brief 根据项目配置启用特定模块，并递归解析依赖
         */
        void ResolveDependencies(const std::vector<std::string>& enabledModules);

        /**
         * @brief 启动所有已解析的模块
         */
        void StartupModules();

        /**
         * @brief 关闭所有模块
         */
        void ShutdownModules();

        /**
         * @brief 每帧更新活动模块
         */
        void UpdateModules(float deltaTime);

        /**
         * @brief 获取已加载的模块实例
         */
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

        // 拓扑排序辅助函数
        void SortModulesByDependency();

        std::map<std::string, ModuleFactory> m_staticFactories;
        std::map<std::string, FModuleInfo> m_discoveredModules;
        std::vector<std::string> m_activeModuleNames;
        std::map<std::string, std::unique_ptr<IModule>> m_loadedModules;
    };

}
