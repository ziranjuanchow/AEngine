#include <catch2/catch_all.hpp>
#include "../src/Core/Engine.h"
#include "../src/Core/Subsystem.h"
#include "../src/Core/Log.h"

class TestSubsystem : public AEngine::IEngineSubsystem {
public:
    virtual void OnStartup() override { m_started = true; }
    virtual void OnShutdown() override { m_started = false; }
    virtual std::string GetName() const override { return "TestSubsystem"; }

    bool IsStarted() const { return m_started; }

private:
    bool m_started = false;
};

TEST_CASE("Engine can manage subsystems", "[Engine]") {
    AEngine::Log::Init();
    auto& engine = AEngine::UEngine::Get();
    
    SECTION("Subsystem registration") {
        auto sub = std::make_unique<TestSubsystem>();
        auto result = engine.RegisterSubsystem(std::move(sub));
        REQUIRE(result.has_value());
    }

    SECTION("Engine startup and shutdown") {
        engine.Init();
        auto* sub = engine.GetSubsystem<TestSubsystem>();
        REQUIRE(sub != nullptr);
        REQUIRE(sub->IsStarted() == true);

        engine.Shutdown();
        REQUIRE(sub->IsStarted() == false);
    }
}
