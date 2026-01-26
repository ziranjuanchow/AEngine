# Track Specification: Cleanup and Documentation

## Goal
Improve codebase readability and documentation clarity to facilitate future refactoring. This track focuses on adding explanatory comments to the rendering core and documenting the current hybrid deferred/forward pipeline architecture.

## Scope
1.  **Code Comments (src/RHI)**:
    -   `FSceneRenderer`: Explain the initialization, FBO creation, and the hardcoded Render loop.
    -   `DeferredGeometryPass`: Explain G-Buffer layout (Location 0, 1, 2).
    -   `DeferredLightingPass`: Explain the light volume rendering and additive blending strategy.
2.  **Documentation (docs/)**:
    -   Create `docs/features/06_Deferred_Pipeline.md`.
    -   Update `docs/Index.md`.
3.  **Refactoring Proposal**:
    -   Create `conductor/tracks/cleanup_and_docs_20260126/refactor_proposal.md` outlining the plan to enforce RHI abstraction.

## Technical Decisions
-   **Doxygen Style**: Use `///` for method documentation and `//` for inline implementation details.
-   **TODO Marking**: Explicitly mark raw GL calls in `SceneRenderer` with `// TODO: Refactor to RHI` to aid the next track.
