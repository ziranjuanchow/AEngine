#include "Engine.h"
#include "Log.h"

namespace AEngine {

    UEngine::UEngine() {}
    UEngine::~UEngine() {
        if (m_initialized) {
            Shutdown();
        }
    }

    UEngine& UEngine::Get() {
        static UEngine instance;
        return instance;
    }

    void UEngine::Init() {
        if (m_initialized) return;

        AE_CORE_INFO("Engine initializing...");
        
        for (auto& subsystem : m_subsystems) {
            AE_CORE_INFO("Starting subsystem: {0}", subsystem->GetName());
            subsystem->OnStartup();
        }

        m_initialized = true;
        AE_CORE_INFO("Engine initialized successfully.");
    }

    void UEngine::Shutdown() {
        if (!m_initialized) return;

        AE_CORE_INFO("Engine shutting down...");
        
        for (auto it = m_subsystems.rbegin(); it != m_subsystems.rend(); ++it) {
            AE_CORE_INFO("Shutting down subsystem: {0}", (*it)->GetName());
            (*it)->OnShutdown();
        }

        m_initialized = false;
        AE_CORE_INFO("Engine shutdown complete.");
    }

    AEngine::expected<void, EngineError> UEngine::RegisterSubsystem(std::unique_ptr<IEngineSubsystem> subsystem) {
        if (m_initialized) {
            return AEngine::unexpected(EngineError::InitializationFailed);
        }

        for (const auto& s : m_subsystems) {
            if (s->GetName() == subsystem->GetName()) {
                return AEngine::unexpected(EngineError::SubsystemAlreadyExists);
            }
        }

        m_subsystems.push_back(std::move(subsystem));
        return {};
    }

}
