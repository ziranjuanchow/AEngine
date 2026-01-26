# Refactoring Proposal: RHI Abstraction Enforcement

## Background
The recent implementation of the Deferred Rendering pipeline introduced significant complexity to `FSceneRenderer`. To move fast, several raw OpenGL calls (`glEnable`, `glBlendFunc`, `glActiveTexture`, `glDrawBuffers`) were hardcoded directly into the high-level renderer logic. This "leakage" violates the project's architectural goal of a backend-agnostic RHI (Rendering Hardware Interface).

## Objective
The goal is to strictly encapsulate all low-level graphics API calls within the `RHI/OpenGL` implementation files, leaving `FSceneRenderer` and high-level Passes completely API-agnostic.

## Proposed Changes

### 1. Extend IRHICommandBuffer
The `IRHICommandBuffer` interface needs to support the state changes currently hardcoded in the renderer.

**New Methods to Add:**
```cpp
struct FRenderState {
    bool DepthTestEnabled = true;
    bool DepthMask = true;
    ERHICompareFunc DepthFunc = ERHICompareFunc::LessEqual;
    bool CullFaceEnabled = true;
    ERHICullMode CullMode = ERHICullMode::Back;
    ERHIWindOrder FrontFace = ERHIWindOrder::CCW;
    bool BlendEnabled = false;
    ERHIBlendFactor BlendSrc = ERHIBlendFactor::One;
    ERHIBlendFactor BlendDst = ERHIBlendFactor::Zero;
};

class IRHICommandBuffer {
    // ... existing methods ...

    // Set complete pipeline state (replaces glEnable/glDisable calls)
    virtual void SetRenderState(const FRenderState& state) = 0;

    // For G-Buffer MRT Setup
    // Replaces glDrawBuffers
    virtual void SetRenderTargetConfiguration(const std::vector<ERHIAttachmentType>& attachments) = 0;

    // For Texture Binding
    // Replaces glActiveTexture + glBindTexture loops
    // Ideally, we move to a slot-based binding system
    virtual void BindTexture(uint32_t slot, IRHITexture* texture) = 0;
    virtual void UnbindAllTextures() = 0; // Helper to reset state
};
```

### 2. Refactor FSceneRenderer
-   **Remove** `#include <glad/glad.h>`.
-   **Replace** `ResetRenderState()` helper with a `FRenderState` struct and a call to `m_cmdBuffer->SetRenderState()`.
-   **Replace** `UnbindAllTextures()` loop with `m_cmdBuffer->UnbindAllTextures()`.

### 3. Refactor Deferred Passes
-   **FDeferredGeometryPass**: Use `SetRenderTargetConfiguration` to define the MRT setup instead of relying on the Framebuffer's internal (and currently hardcoded) setup or raw GL calls.
-   **FDeferredLightingPass**: Use `SetRenderState` to enable Additive Blending (`One`, `One`) and `CullFront`.

### 4. OpenGL Backend Implementation
-   Implement the new virtual methods in `OpenGLCommandBuffer.cpp`.
-   Ensure state caching (optional but recommended) to avoid redundant driver calls.

## Benefits
-   **Portability**: Paves the way for Vulkan/DX12 backends.
-   **Readability**: High-level logic reads like a script ("Set State -> Draw") rather than a driver manual.
-   **Stability**: Centralized state management reduces "state leakage" bugs where a previous pass accidentally affects the next one.
