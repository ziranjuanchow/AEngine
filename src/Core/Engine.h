#pragma once

#include <vector>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include "Subsystem.h"

namespace AEngine {

    // Polyfill for C++23 std::expected (used for error handling without exceptions)
    template <typename T, typename E>
    using expected = tl::expected<T, E>;
    
    template <typename E>
    using unexpected = tl::unexpected<E>;

    enum class EngineError {
        SubsystemAlreadyExists,
        SubsystemNotFound,
        InitializationFailed
    };

    /// @brief The Core Engine Class (Singleton).
    /// Manages the lifecycle of the application and all Engine Subsystems.
    /// Follows a simplified Service Locator pattern via RegisterSubsystem/GetSubsystem.
    class UEngine {
    public:
        UEngine();
        ~UEngine();

        /// @brief Access the global engine instance.
        static UEngine& Get();

        /// @brief Initialize all registered subsystems.
        /// Called once at application startup.
        void Init();

        /// @brief Shutdown all subsystems in reverse order.
        void Shutdown();

        /// @brief Registers a new subsystem to be managed by the engine.
        /// Subsystems are initialized in the order they are registered.
        /// @param subsystem Unique pointer to the subsystem instance.
        AEngine::expected<void, EngineError> RegisterSubsystem(std::unique_ptr<IEngineSubsystem> subsystem);
        
        /// @brief Retrieves a registered subsystem by type.
        /// @return Pointer to the subsystem, or nullptr if not found.
        template<typename T>
        T* GetSubsystem() {
            for (auto& subsystem : m_subsystems) {
                if (T* ptr = dynamic_cast<T*>(subsystem.get())) {
                    return ptr;
                }
            }
            return nullptr;
        }

    private:
        std::vector<std::unique_ptr<IEngineSubsystem>> m_subsystems;
        bool m_initialized = false;
    };

}
