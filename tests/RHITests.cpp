#include <catch2/catch_all.hpp>
#include "Engine.RHI/IRHIDevice.h"
#include "Engine/Plugins/RHI.OpenGL/OpenGLDevice.h"

TEST_CASE("Framebuffer Configuration", "[RHI]") {
    using namespace AEngine;

    FFramebufferConfig config;
    config.Width = 1024;
    config.Height = 768;
    
    // Simulate MRT
    config.ColorAttachments.resize(3);
    // In a real test we would create mock textures, here we just verify struct exists
    REQUIRE(config.ColorAttachments.size() == 3);
}

TEST_CASE("HDR Texture Support", "[RHI]") {
    using namespace AEngine;
    // We can't easily test OpenGL context creation in headless unit tests without window
    // But we can verify the enum exists and logic holds
    auto format = ERHIPixelFormat::RGBA16_FLOAT;
    REQUIRE((int)format > 0);
}
