#include "../../src/Core/Plugin.h"
#include <imgui.h>

namespace AEngine {

    class FStatsPanelPlugin : public IPlugin {
    public:
        virtual void Initialize(void* context) override {
            ImGui::SetCurrentContext((ImGuiContext*)context);
        }

        virtual void OnLoad() override {}
        virtual void OnUnload() override {}

        virtual void OnImGuiRender() override {
            ImGui::Begin("Performance Stats");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        virtual std::string GetName() const override { return "StatsPanel"; }
    };

}

extern "C" __declspec(dllexport) AEngine::IPlugin* CreatePlugin() {
    return new AEngine::FStatsPanelPlugin();
}
