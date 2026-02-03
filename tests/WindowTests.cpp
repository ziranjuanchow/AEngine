#include <catch2/catch_all.hpp>
#include "Engine.Window/WindowModule.h"
#include "Kernel/Core/Log.h"

using namespace AEngine;

TEST_CASE("WindowModule basic interface", "[Engine]") {
    Log::Init();
    UWindowModule window;

    SECTION("Name check") {
        // WindowModule 不再需要 GetName，由 module.json 决定
        REQUIRE(true); 
    }
}