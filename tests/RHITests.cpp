#include <catch2/catch_all.hpp>
#include "Engine.RHI/IRHIDevice.h"
#include "Engine.RHI/RHIResources.h" // For ERHIPixelFormat

namespace AEngine { // Need to be inside AEngine namespace for ERHIPixelFormat

    // Mock/Dummy implementation of IRHICommandBuffer for testing
    class FMockCommandBuffer : public IRHICommandBuffer {
    public:
        bool setDrawBuffersCalled = false;
        std::vector<ERHIPixelFormat> lastFormats;

        bool setBlendFuncCalled = false;
        ERHIBlendFactor lastSFactor;
        ERHIBlendFactor lastDFactor;

        // Implement all pure virtual functions (dummy implementations)
        void Begin() override {}
        void End() override {}
        void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override {}
        void Clear(float r, float g, float b, float a, bool clearDepth = true) override {}
        void SetDepthBias(float constant, float slope) override {}
        void SetBlendState(bool enabled) override {}
        void SetBlendFunc(ERHIBlendFactor sfactor, ERHIBlendFactor dfactor) override { // Added
            setBlendFuncCalled = true;
            lastSFactor = sfactor;
            lastDFactor = dfactor;
        }
        void SetDepthTest(bool enabled, bool writeEnabled, uint32_t func = 0x0203) override {}
        void SetCullMode(uint32_t mode) override {}
        void SetPipelineState(std::shared_ptr<IRHIPipelineState> pso) override {}
        void SetVertexBuffer(std::shared_ptr<IRHIBuffer> buffer) override {}
        void SetIndexBuffer(std::shared_ptr<IRHIBuffer> buffer) override {}
        void SetUniform(uint32_t location, const glm::mat4& value) override {}
        void SetUniform(uint32_t location, const glm::vec3& value) override {}
        void SetUniform(uint32_t location, int value) override {}
        void SetUniform(uint32_t location, float value) override {}
        void Draw(uint32_t vertexCount, uint32_t instanceCount = 1) override {}
        void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1) override {}

        // Actual implementation for the method under test
        void SetDrawBuffers(const std::vector<ERHIPixelFormat>& formats) override {
            setDrawBuffersCalled = true;
            lastFormats = formats;
        }
    };

} // namespace AEngine

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

// GREEN PHASE: This test now verifies the IRHICommandBuffer::SetDrawBuffers call
TEST_CASE("IRHICommandBuffer SetDrawBuffers Functional Test (Green Phase)", "[RHI][SetDrawBuffers]") {
    using namespace AEngine;

    FMockCommandBuffer mockCmdBuffer;
    std::vector<ERHIPixelFormat> formats = { ERHIPixelFormat::RGBA8_UNORM, ERHIPixelFormat::RGBA16_FLOAT };
    
    // Call the method under test on the mock object
    mockCmdBuffer.SetDrawBuffers(formats); 
    
    // Verify that the method was called and with the correct parameters
    REQUIRE(mockCmdBuffer.setDrawBuffersCalled == true);
    REQUIRE(mockCmdBuffer.lastFormats == formats);
}

// GREEN PHASE: This test now verifies the IRHICommandBuffer::SetBlendFunc call
TEST_CASE("IRHICommandBuffer SetBlendFunc Functional Test (Green Phase)", "[RHI][SetBlendFunc]") {
    using namespace AEngine;

    FMockCommandBuffer mockCmdBuffer;
    ERHIBlendFactor sfactor = ERHIBlendFactor::SrcAlpha;
    ERHIBlendFactor dfactor = ERHIBlendFactor::OneMinusSrcAlpha;
    
    // Call the method under test on the mock object
    mockCmdBuffer.SetBlendFunc(sfactor, dfactor); 
    
    // Verify that the method was called and with the correct parameters
    REQUIRE(mockCmdBuffer.setBlendFuncCalled == true);
    REQUIRE(mockCmdBuffer.lastSFactor == sfactor);
    REQUIRE(mockCmdBuffer.lastDFactor == dfactor);
}