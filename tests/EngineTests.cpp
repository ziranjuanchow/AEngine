#include <catch2/catch_all.hpp>
#include "Kernel/ModuleManager/ModuleManager.h"
#include "Kernel/Core/Log.h"

using namespace AEngine;

class FTestModule : public IModule {
public:
    virtual void OnStartup() override { m_started = true; }
    virtual void OnShutdown() override { m_started = false; }
    bool IsStarted() const { return m_started; }
private:
    bool m_started = false;
};

TEST_CASE("ModuleManager can manage modules", "[Kernel]") {
    Log::Init();
    auto& mm = FModuleManager::Get();
    
    SECTION("Module registration and lifecycle") {
        mm.RegisterStaticModule("Test.Module", []() { return std::make_unique<FTestModule>(); });
        
        // 我们手动模拟发现过程
        // 在真实场景中会扫描 JSON，这里我们直接注入
        mm.ResolveDependencies({"Test.Module"});
        mm.StartupModules();

        auto* mod = mm.GetModule<FTestModule>("Test.Module");
        REQUIRE(mod != nullptr);
        REQUIRE(mod->IsStarted() == true);

        mm.ShutdownModules();
        // 注意：ShutdownModules 会清除 map
        REQUIRE(mm.GetModule<FTestModule>("Test.Module") == nullptr);
    }
}