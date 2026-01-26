# Track Implementation Plan - Cleanup and Documentation

## Phase 1: Code Annotations [Completed]

- [x] Task: Add comments to FSceneRenderer (Header & Cpp)
    - Explain the "Hybrid Pipeline" concept in the class header.
    - Document `Init` (FBO creation dependencies).
    - Annotate the `Render` loop step-by-step (G-Buffer -> Lighting -> Blit -> Forward).
    - Mark raw GL calls with `TODO: Refactor to RHI`.
- [x] Task: Add comments to Deferred Passes
    - Annotate `DeferredGeometryPass` (MRT layout).
    - Annotate `DeferredLightingPass` (Light volumes, blending).

## Phase 2: Documentation Update [Completed]

- [x] Task: Create Pipeline Documentation
    - Write `docs/features/06_Deferred_Pipeline.md` describing the G-Buffer layout and Pass order.
- [x] Task: Update Index
    - Link new doc in `docs/Index.md`.

## Phase 3: Refactoring Proposal [Completed]

- [x] Task: Draft Refactoring Plan
    - Create `refactor_proposal.md` in the track folder.
    - Detail the steps to abstract `glDrawBuffers`, `glBlendFunc`, and `glBlitFramebuffer` into `IRHICommandBuffer`.

## Phase 4: Comprehensive Annotation [Completed]

- [x] Task: Annotate RHI Interfaces
    - `IRHIDevice.h`, `RHIResources.h`, `RHIDefinitions.h`.
- [x] Task: Annotate Shaders
    - `DeferredGeometry.vert/frag` (Explain MRT layout).
    - `DeferredLighting.vert/frag` (Explain PBR math and Light Volume logic).
    - Fix debug output in `DeferredLighting.frag`.
- [x] Task: Annotate Core
    - `UEngine` (Singleton and Subsystem management).
