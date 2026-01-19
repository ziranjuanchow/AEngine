#pragma once

#include <vector>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include "Subsystem.h"

namespace AEngine {

    // Polyfill for C++23 std::expected
    template <typename T, typename E>
    using expected = tl::expected<T, E>;
    
    template <typename E>
    using unexpected = tl::unexpected<E>;

    enum class EngineError {
        SubsystemAlreadyExists,
        SubsystemNotFound,
        InitializationFailed
    };

    class UEngine {
    public:
        UEngine();
        ~UEngine();

        static UEngine& Get();

        void Init();
        void Shutdown();

        AEngine::expected<void, EngineError> RegisterSubsystem(std::unique_ptr<IEngineSubsystem> subsystem);
        
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
