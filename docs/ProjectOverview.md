# AEngine - Project Overview

## 1. 核心目标 (Project Vision)
AEngine 是一款专为游戏引擎开发者设计的高性能、高度模块化实时渲染器。它采用现代 C++ (C++20) 开发，旨在提供一个灵活且健壮的框架，用于前沿图形学算法的研究与实现。

**核心理念：**
- **模块化 (Modularity)**：核心架构微内核化，渲染功能通过插件扩展。
- **可插拔 RHI (Pluggable RHI)**：设计支持 OpenGL, Vulkan, DirectX 的动态切换（目前已实现 OpenGL 4.5+ 后端）。
- **工程化 (Engineering)**：强调清晰的依赖管理 (vcpkg)、构建系统 (CMake) 和代码规范。

## 2. 目标用户 (Target Audience)
- **图形程序员**：需要一个干净的沙盒来验证 PBR、光线追踪或 GI 算法。
- **引擎开发者**：需要参考现代 RHI 设计或子系统架构。
- **技术美术 (TA)**：需要一个可编程的渲染管线环境。

## 3. 系统架构 (System Architecture)

### 3.1 核心层 (Core)
- **UEngine**：全局单例，管理所有子系统的生命周期。
- **IEngineSubsystem**：所有核心模块（如窗口、渲染、输入）的基类接口。
- **APluginManager**：负责动态加载 DLL 插件，支持热插拔架构。
- **LogSystem**：基于 spdlog 的高性能异步日志系统。

### 3.2 渲染抽象层 (RHI)
AEngine 采用显式 RHI 设计，屏蔽底层 API 差异：
- **IRHIDevice**：资源创建工厂（Buffer, Texture, Shader）。
- **IRHICommandBuffer**：渲染指令录制接口。
- **Backend**：目前实现了 `FOpenGLDevice`，基于 OpenGL 4.6 (Core Profile) 和 DSA (Direct State Access) 特性。

### 3.3 资源与管线
- **Shader System**：集成 `glslang`，支持将 GLSL 编译为 SPIR-V 中间语言。支持 `#include` 模块化开发。
- **Render Graph**：基于 Pass 的渲染管线组织方式，将场景数据解耦为独立的渲染任务（如 `FForwardLitPass`）。
- **Asset Loader**：集成 `stb_image` 和 `assimp`，支持 HDR 环境贴图和基础模型加载。

## 4. 技术栈 (Tech Stack)
- **语言**：C++ 20
- **构建系统**：CMake 3.20+
- **依赖管理**：vcpkg (Manifest Mode)
- **核心库**：
    - **UI**: Dear ImGui (Docking Branch)
    - **Window**: GLFW 3.4
    - **Math**: GLM
    - **Reflection**: RTTR
    - **Graphics**: Glad (Loader), glslang (Compiler), SPIRV-Cross
    - **Logging**: spdlog
