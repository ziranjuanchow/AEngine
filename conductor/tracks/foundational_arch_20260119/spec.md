# Track 规格说明 - 基础架构与 OpenGL RHI 验证

## 1. 概述
本 Track 旨在为 AEngine 建立核心运行环境和第一个图形渲染基准。通过实现模块化的子系统和插件机制，确保引擎具备高度的可扩展性。

## 2. 核心目标
- **核心子系统**：完成日志 (spdlog)、输入 (GLFW) 和资源管理基础。
- **RHI 抽象层**：实现针对 OpenGL 4.6 的初版抽象，支持基础渲染指令。
- **模块化插件系统**：实现 DLL/SO 动态加载机制，并提供一个 ImGui 扩展插件。
- **Shader 系统**：集成 glslang，支持将模块化 Shader (含 #include 处理) 编译为 SPIR-V。
- **场景管理**：实现基础场景树，支持节点间的 TRS 变换。

## 3. 技术约束
- 语言标准：C++20/23。
- 依赖管理：vcpkg。
- 图形 API：OpenGL 4.6 (Core Profile)。
- 错误处理：统一使用 `std::expected` (或兼容实现)。
