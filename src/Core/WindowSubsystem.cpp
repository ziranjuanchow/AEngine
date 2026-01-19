#include <glad/glad.h>
#include "WindowSubsystem.h"
#include "Log.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace AEngine {

    static void GLFWErrorCallback(int error, const char* description) {
        AE_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    void UWindowSubsystem::OnStartup() {
        AE_CORE_INFO("Starting WindowSubsystem...");

        if (!glfwInit()) {
            AE_CORE_CRITICAL("Could not initialize GLFW!");
            return;
        }

        glfwSetErrorCallback(GLFWErrorCallback);

        // For OpenGL 4.5 Core Profile (More compatible)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow((int)m_props.Width, (int)m_props.Height, m_props.Title.c_str(), nullptr, nullptr);
        if (!m_window) {
            AE_CORE_CRITICAL("Could not create window! Check if your GPU supports OpenGL 4.5");
            return;
        }

        glfwMakeContextCurrent(m_window);
        glfwSwapInterval(1); // Enable VSync

        // Initialize Glad
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            AE_CORE_CRITICAL("Failed to initialize Glad!");
            std::exit(-1);
        }
        AE_CORE_INFO("OpenGL Info:");
        AE_CORE_INFO("  Vendor: {0}", (const char*)glGetString(GL_VENDOR));
        AE_CORE_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
        AE_CORE_INFO("  Version: {0}", (const char*)glGetString(GL_VERSION));

        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Temporarily disabled for debugging

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 460");

        AE_CORE_INFO("WindowSubsystem started successfully.");
    }

    void UWindowSubsystem::OnShutdown() {
        AE_CORE_INFO("Shutting down WindowSubsystem...");

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (m_window) {
            glfwDestroyWindow(m_window);
        }
        glfwTerminate();
    }

    void UWindowSubsystem::Update() {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    bool UWindowSubsystem::ShouldClose() const {
        return glfwWindowShouldClose(m_window);
    }

}
