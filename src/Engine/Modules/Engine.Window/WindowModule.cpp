#include "WindowModule.h"
#include <glad/glad.h>
#include "Kernel/Core/Log.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace AEngine {

    static void GLFWErrorCallback(int error, const char* description) {
        AE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    void UWindowModule::OnStartup() {
        AE_CORE_INFO("Starting WindowModule...");

        if (!glfwInit()) {
            AE_CORE_CRITICAL("Could not initialize GLFW!");
            return;
        }

        glfwSetErrorCallback(GLFWErrorCallback);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow((int)m_props.Width, (int)m_props.Height, m_props.Title.c_str(), nullptr, nullptr);
        if (!m_window) {
            AE_CORE_CRITICAL("Could not create window!");
            return;
        }

        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1); 

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            AE_CORE_CRITICAL("Failed to initialize Glad!");
            return;
        }

        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 450");

        AE_CORE_INFO("WindowModule started successfully.");
    }

    void UWindowModule::OnShutdown() {
        AE_CORE_INFO("Shutting down WindowModule...");
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (m_window) {
            glfwDestroyWindow(m_window);
        }
        glfwTerminate();
    }

    void UWindowModule::OnUpdate(float deltaTime) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    bool UWindowModule::ShouldClose() const {
        return glfwWindowShouldClose(m_window);
    }

}
