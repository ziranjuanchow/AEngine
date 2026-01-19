#pragma once

#include "Subsystem.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>

namespace AEngine {

    struct FWindowProps {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        FWindowProps(const std::string& title = "AEngine",
                     uint32_t width = 1280,
                     uint32_t height = 720)
            : Title(title), Width(width), Height(height) {}
    };

    class UWindowSubsystem : public IEngineSubsystem {
    public:
        virtual void OnStartup() override;
        virtual void OnShutdown() override;
        virtual std::string GetName() const override { return "WindowSubsystem"; }

        void Update();
        bool ShouldClose() const;

        GLFWwindow* GetNativeWindow() const { return m_window; }

    private:
        GLFWwindow* m_window = nullptr;
        FWindowProps m_props;
    };

}
