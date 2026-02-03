#include <catch2/catch_all.hpp>
#include "Kernel/Core/Log.h"

TEST_CASE("LogSubsystem can log messages", "[Log]") {
    // This is a bit hard to test without capturing stdout, 
    // but we can test if the logger is initialized.
    AEngine::Log::Init();
    
    REQUIRE(AEngine::Log::GetCoreLogger() != nullptr);
    REQUIRE(AEngine::Log::GetClientLogger() != nullptr);
}
