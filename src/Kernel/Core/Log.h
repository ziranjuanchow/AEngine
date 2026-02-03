#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include "Module.h"

namespace AEngine {

    class Log {
    public:
        static AE_API void Init();

        static AE_API std::shared_ptr<spdlog::logger>& GetCoreLogger();
        static AE_API std::shared_ptr<spdlog::logger>& GetClientLogger();

    private:
        static std::shared_ptr<spdlog::logger> s_coreLogger;
        static std::shared_ptr<spdlog::logger> s_clientLogger;
    };

}

// Core log macros
#define AE_CORE_TRACE(...)    ::AEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define AE_CORE_INFO(...)     ::AEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define AE_CORE_WARN(...)     ::AEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define AE_CORE_ERROR(...)    ::AEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define AE_CORE_CRITICAL(...) ::AEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define AE_TRACE(...)         ::AEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define AE_INFO(...)          ::AEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define AE_WARN(...)          ::AEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define AE_ERROR(...)         ::AEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define AE_CRITICAL(...)      ::AEngine::Log::GetClientLogger()->critical(__VA_ARGS__)