#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <vector>

namespace AEngine {

    std::shared_ptr<spdlog::logger> Log::s_coreLogger;
    std::shared_ptr<spdlog::logger> Log::s_clientLogger;

    void Log::Init() {
        if (spdlog::get("AENGINE")) return;

        std::vector<spdlog::sink_ptr> logSinks;
        logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        logSinks.emplace_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/AEngine.log", 1024 * 1024 * 5, 10));

        logSinks[0]->set_pattern("%^[%T] %n: %v%$");
        logSinks[1]->set_pattern("[%T] [%l] %n: %v");

        s_coreLogger = std::make_shared<spdlog::logger>("AENGINE", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_coreLogger);
        s_coreLogger->set_level(spdlog::level::trace);
        s_coreLogger->flush_on(spdlog::level::trace);

        s_clientLogger = std::make_shared<spdlog::logger>("APP", begin(logSinks), end(logSinks));
        spdlog::register_logger(s_clientLogger);
        s_clientLogger->set_level(spdlog::level::trace);
        s_clientLogger->flush_on(spdlog::level::trace);
    }

}
