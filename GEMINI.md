# AEngine Project Context for AI

## 1. Build System & Environment (CRITICAL)
- **Use `build.bat`**: ALWAYS use `build.bat` for building and testing. The user's system has an environment conflict where MSYS2's CMake often overrides the standard Windows/Visual Studio CMake. `build.bat` is designed to sanitize the PATH and activate the correct Visual Studio 2026 (v18) environment.
- **系统环境变量修改同意**: AI 代理在执行任何修改**系统全局环境变量**的操作之前，必须征求用户的明确同意。临时的（仅限当前 shell 会话）环境变量修改，AI 代理可自行决定。
- **Generator**: Prefer **Ninja** (if available in VS environment) or **Visual Studio 17 2022** (pointing to VS 2026/v18 toolchain).
- **Dependency Manager**: **vcpkg** in Manifest mode (`vcpkg.json`).

## 2. Coding Standards
- **Naming Convention** (UE-Style):
  - `I` for Interfaces (e.g., `IEngineSubsystem`, `IRHIDevice`).
  - `F` for Structs/Classes (e.g., `FSceneNode`, `FOpenGLDevice`).
  - `U` for Objects/Subsystems (e.g., `UEngine`, `UWindowSubsystem`).
  - `A` for Managers (e.g., `APluginManager`).
- **C++ Standard**: C++20.
- **Error Handling**: Use `tl::expected` (aliased as `AEngine::expected` in `src/Core/Engine.h`). Avoid using C++ exceptions.
- **Static Analysis**: All C++ code must pass `clang-tidy` checks. The configuration is integrated into `CMakeLists.txt` using `bugprone-*`, `performance-*`, `modernize-*`, and `readability-*` checks.

## 3. Architecture Overview
- **RHI (Rendering Hardware Interface)**: An abstraction layer over OpenGL 4.5+ (currently). Uses `glad` as the OpenGL loader.
- **Shader System**: Shaders are written in GLSL and compiled to SPIR-V using `glslang`. The compiler includes a custom `FGLSLIncluder` to support the `#include` directive.
- **Render Pipeline**: Uses a **Render Graph** architecture. The main rendering pass currently is `FForwardLitPass`, which implements PBR direct lighting (Cook-Torrance BRDF).
- **UI**: Deeply integrated with **Dear ImGui (Docking Branch)**. Use `ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode)` to ensure the background color is cleared properly.

## 4. Key Components
- **Engine Core**: `UEngine` manages the lifecycle of all registered `IEngineSubsystem` instances.
- **Plugin System**: `APluginManager` allows for dynamic loading of DLLs. Plugins must implement `IPlugin` and export a `CreatePlugin` function.
- **Geometry**: `FGeometryUtils` provides helper functions to generate basic shapes (e.g., Spheres) with full `FVertex` data.

## 5. Development Workflow
- Follow the **Conductor** methodology.
- Always run `.\build.bat` before submitting any task to verify that the compilation and unit tests (Catch2) pass.
- **Documentation Requirement**: After completing each track (feature, bug fix, or major change), the AI must update or create relevant documentation in `docs/features/` describing the implementation. Ensure `docs/Index.md` is updated to reflect these changes.
- **Technical Decision Tracking**: Every `spec.md` for a new track MUST include a "Technical Decisions" section. This section should document the chosen solution, the rationale behind it, and the alternative solutions that were considered but rejected (along with reasons for rejection). This maintains a clear history of the project's architectural evolution.
