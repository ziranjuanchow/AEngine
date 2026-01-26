# Deferred Rendering Pipeline (Hybrid)

## Overview
AEngine uses a hybrid rendering pipeline that combines **Deferred Shading** for opaque geometry and **Forward Rendering** for transparent objects and debug primitives. This approach allows for efficient handling of many dynamic lights while maintaining support for complex materials and transparency.

## Architecture

The rendering process is orchestrated by `FSceneRenderer` and executed via `FRenderGraph`.

### Render Pass Flow

1.  **Shadow Pass** (Forward)
    -   **Input**: Scene Geometry (Opaque)
    -   **Output**: Shadow Map (Depth Texture)
    -   **Details**: Renders the scene from the directional light's perspective. Currently supports a single cascade.

2.  **G-Buffer Pass** (Deferred)
    -   **Input**: Scene Geometry (Opaque)
    -   **Output**: G-Buffer (Multiple Render Targets)
    -   **Layout**:
        -   `RT0` (RGBA8): Albedo (RGB) + Roughness (A)
        -   `RT1` (RGBA16F): World Space Normal (RGB) + Metallic (A - *Reserved*)
        -   `RT2` (RGBA8): Emissive (RGB) + AO (A - *Reserved*)
        -   `DepthStencil` (D24S8): Shared Depth Buffer

3.  **Lighting Pass** (Deferred)
    -   **Input**: G-Buffer Textures, Point Light Data
    -   **Output**: HDR Color Buffer
    -   **Technique**: **Light Volume Rendering**.
        -   For each point light, a sphere geometry is rendered.
        -   The shader reconstructs World Position from Depth.
        -   Lighting is calculated (PBR Cook-Torrance) and added to the HDR buffer using **Additive Blending** (`glBlendFunc(GL_ONE, GL_ONE)`).
        -   *Optimization*: Cull Front faces to allow camera to be inside the light volume.

4.  **Forward Pass** (Forward)
    -   **Input**: Scene Geometry (Transparent/Debug), Skybox
    -   **Output**: HDR Color Buffer (Overlay)
    -   **Details**:
        -   Reuses the Depth Buffer from the G-Buffer Pass (Read-Only Depth Test).
        -   Renders objects that cannot be handled by deferred shading (e.g., glass, particles).

5.  **Post Process Pass**
    -   **Input**: HDR Color Buffer
    -   **Output**: Screen (LDR)
    -   **Details**: Applies Tone Mapping (ACES) and Gamma Correction.

## Key Classes

-   `FSceneRenderer`: The high-level manager that constructs the graph and managing FBO lifecycles.
-   `FDeferredGeometryPass`: Handles the MRT setup for G-Buffer generation.
-   `FDeferredLightingPass`: Manages the light volume geometry and additive blending state.
-   `IRHICommandBuffer`: (Incomplete) Abstraction for GPU commands. Currently, `FSceneRenderer` leaks some raw OpenGL calls.

## Future Improvements (Refactoring Goals)
1.  **RHI Abstraction**: Move all `glActiveTexture`, `glBlendFunc`, and `glDrawBuffers` calls into `IRHICommandBuffer`.
2.  **Uniform Buffers**: Replace individual `glUniform` calls with `UniformBuffer` (UBO) for camera and light data.
3.  **Tile-Based Lighting**: Investigate Tile/Cluster-based deferred shading for further optimization.
