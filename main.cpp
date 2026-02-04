#include "Core/FApplication.h"

// 通用启动器
class AEngineLauncher : public AEngine::FApplication {
public:
    AEngineLauncher() : FApplication({ .Name = "AEngine Launcher" }) {}
};

AEngine::FApplication* AEngine::CreateApplication() { return new AEngineLauncher(); }

int main() {
    auto* app = AEngine::CreateApplication();
    app->Run();
    delete app;
    return 0;
}
