#pragma once

#include <memory>
#include <string>
#include "Engine.h"
#include "WindowSubsystem.h"

namespace AEngine {

    struct FApplicationSpecs {
        std::string Name = "AEngine App";
        uint32_t Width = 1280;
        uint32_t Height = 720;
    };

    class FApplication {
    public:
        FApplication(const FApplicationSpecs& specs);
        virtual ~FApplication();

        void Run();

        virtual void OnInit() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnShutdown() {}
        virtual void OnImGuiRender() {}

    protected:
        FApplicationSpecs m_specs;
        bool m_running = true;
        UWindowSubsystem* m_window = nullptr; // Raw ptr, owned by Engine
    };

    // To be defined in CLIENT
    FApplication* CreateApplication();

}
