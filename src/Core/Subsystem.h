#pragma once

#include <string>

namespace AEngine {

    /**
     * @brief Interface for engine subsystems.
     * 
     * Subsystems are core modules that are managed by the UEngine lifecycle.
     * Examples: WindowSubsystem, RenderSubsystem, InputSubsystem.
     */
    class IEngineSubsystem {
    public:
        virtual ~IEngineSubsystem() = default;

        /**
         * @brief Called when the engine initializes.
         * Initialize resources and dependencies here.
         */
        virtual void OnStartup() = 0;

        /**
         * @brief Called when the engine shuts down.
         * Release resources here.
         */
        virtual void OnShutdown() = 0;

        /**
         * @brief Gets the unique name of the subsystem.
         * @return The name string.
         */
        virtual std::string GetName() const = 0;
    };

}
