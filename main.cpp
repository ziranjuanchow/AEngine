#include <iostream>
#include "Core/Log.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <rttr/registration.h>
#include <assimp/Importer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <imgui.h>

int main() {
    AEngine::Log::Init();
    AE_CORE_INFO("AEngine Initialized!");
    
    if (!glfwInit()) {
        AE_CORE_ERROR("Failed to initialize GLFW!");
        return -1;
    }
    
    AE_CORE_INFO("GLFW Initialized!");
    
    glfwTerminate();
    return 0;
}
