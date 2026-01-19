#include <catch2/catch_all.hpp>
#include "../src/Core/WindowSubsystem.h"
#include "../src/Core/Log.h"

TEST_CASE("WindowSubsystem lifecycle", "[Window]") {
    AEngine::Log::Init();
    AEngine::UWindowSubsystem window;

    SECTION("Startup and Shutdown") {
        // Note: This might fail in CI without a GPU/Display,
        // but should pass on local developer machines.
        // window.OnStartup();
        // REQUIRE(window.GetNativeWindow() != nullptr);
        // window.OnShutdown();
    }
}
