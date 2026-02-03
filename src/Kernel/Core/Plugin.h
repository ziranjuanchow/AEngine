#pragma once

#include <string>
#include <memory>

namespace AEngine {

    /**
     * @brief Interface for dynamic plugins.
     * 
     * Plugins are dynamically loaded libraries (DLLs) that extend engine functionality.
     */
    class IPlugin {
    public:
        virtual ~IPlugin() = default;

        /**
         * @brief Called immediately after the plugin is loaded.
         * @param context Pointer to the ImGui context for synchronization.
         */
        virtual void Initialize(void* context) = 0;

        /**
         * @brief Called after initialization.
         */
        virtual void OnLoad() = 0;

        /**
         * @brief Called before the plugin is unloaded.
         */
        virtual void OnUnload() = 0;

        /**
         * @brief Called during the ImGui rendering phase.
         */
        virtual void OnImGuiRender() = 0;

        /**
         * @brief Gets the unique name of the plugin.
         */
        virtual std::string GetName() const = 0;
    };

    // Plugin entry point signature
    typedef IPlugin* (*CreatePluginFunc)();

}
