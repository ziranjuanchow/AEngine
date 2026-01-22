#include <catch2/catch_all.hpp>
#include "../src/RHI/IRHIDevice.h"
#include <vector>

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
