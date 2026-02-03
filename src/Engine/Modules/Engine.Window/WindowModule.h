#pragma once

#include "Kernel/Core/Module.h"
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

    /**
     * @brief 窗口模块
     */
    class UWindowModule : public IModule {
    public:
        // --- IModule Interface ---
        virtual void OnStartup() override;
        virtual void OnShutdown() override;
        virtual void OnUpdate(float deltaTime) override;
        // -------------------------

        bool ShouldClose() const;
        GLFWwindow* GetNativeWindow() const { return m_window; }

    private:
        GLFWwindow* m_window = nullptr;
        FWindowProps m_props;
    };

}
