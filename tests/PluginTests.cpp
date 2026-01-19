#include <catch2/catch_all.hpp>
#include "../src/Core/PluginManager.h"
#include "../src/Core/Log.h"

TEST_CASE("PluginManager can attempt to load plugins", "[Plugin]") {
    AEngine::Log::Init();
    auto& pm = AEngine::APluginManager::Get();

    SECTION("Loading non-existent plugin") {
        auto result = pm.LoadPlugin("NonExistent.dll");
        REQUIRE(result.has_value() == false);
    }
}
