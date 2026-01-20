# 技术栈 - AEngine

## 1. 核心语言与编译器
- **语言**：C++20/23
- **构建系统**：CMake (3.20+)
- **包管理器**：vcpkg (用于管理跨平台第三方依赖)

## 2. 图形 API 与 RHI
- **多 API 支持**：
    - **Vulkan** (现代高性能图形 API)
    - **DirectX 12** (Windows 平台首选)
    - **OpenGL 4.6** (跨平台、成熟的图形 API)
- **窗口与输入管理**：GLFW (跨平台窗口管理与输入响应)

## 3. 数学与工具库
- **图形数学** (混合模式)：
    - **GLM**：用于 Vulkan 和 OpenGL 分支。
    - **DirectXMath**：用于 DirectX 12 分支，确保 SIMD 优化。
- **反射系统**：rttr (Run Time Type Reflection)，用于实现子系统和插件的动态属性管理。
- **日志系统**：spdlog (高性能、格式化的 C++ 日志库)。
- **错误处理**：tl-expected (用于 C++20 环境下的 std::expected polyfill)。

## 4. 资源加载 (可扩展模块化)
- **图像加载**：stb_image (轻量化、默认支持)。
- **模型加载**：
    - **核心默认**：assimp (已集成，支持多种 3D 格式，包括 OBJ, FBX, GLTF)。
    - **扩展预留**：接口设计支持后续集成 tinygltf (glTF 原生支持) 和 FBX SDK。
- **着色器编译**：glslang (用于将 GLSL 编译为 SPIR-V)，spirv-cross (用于字节码分析)。
- **几何体生成**：内置 FGeometryUtils，支持程序化生成球体等基础形状。

## 5. UI 框架
- **编辑器 UI**：Dear ImGui (集成到各图形 API 的后端，用于构建极简风格编辑器)。
