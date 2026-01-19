# Track 实施计划 - foundational_arch_20260119

## Phase 1: 基础子系统与工程环境搭建 [checkpoint: 2ca0b23]

- [x] Task: 初始化工程结构与 vcpkg 依赖环境
    - [x] 编写 CMakeLists.txt，配置 C++20/23 标准
    - [x] 配置 vcpkg.json，包含 spdlog, glfw3, glm, rttr, assimp, stb
    - 已创建基础 main.cpp 用于验证。
- [x] Task: 实现日志子系统 (LogSubsystem)
    - [x] [TDD] 编写日志输出测试用例 (tests/LogTests.cpp)
    - [x] 封装 spdlog，实现 `AELog` 宏与级别控制 (src/Core/Log.h/cpp)
    - 已在 main.cpp 中集成验证。
- [x] Task: 实现核心引擎类 (EngineCore) 与子系统管理
    - [x] [TDD] 编写子系统注册与生命周期测试 (tests/EngineTests.cpp)
    - [x] 实现 `IEngineSubsystem` 接口与 `ASubsystemManager` (src/Core/Subsystem.h, src/Core/Engine.h/cpp)
- [ ] Task: Conductor - 用户手册验证 'Phase 1: 基础子系统与工程环境搭建' (Protocol in workflow.md)

## Phase 2: 插件系统原型与 ImGui 集成

- [ ] Task: 实现插件加载机制 (PluginSystem)
    - [ ] [TDD] 编写跨平台动态库加载测试
    - [ ] 实现 `APluginManager`，支持 `Load/Unload`
- [ ] Task: 集成 Dear ImGui 与 GLFW 窗口子系统
    - [ ] [TDD] 编写窗口创建与事件响应测试
    - [ ] 实现 `WindowSubsystem`，并初始化 ImGui OpenGL3 后端
- [ ] Task: 实现第一个 ImGui 调试插件
    - [ ] [TDD] 验证插件逻辑被成功调用
    - [ ] 编写一个简单的性能监控面板插件
- [ ] Task: Conductor - 用户手册验证 'Phase 2: 插件系统原型与 ImGui 集成' (Protocol in workflow.md)

## Phase 3: RHI 抽象层 (OpenGL) 与场景树

- [ ] Task: 定义 RHI 基础接口 (IRHIDevice, IRHICommandBuffer)
    - [ ] [TDD] 编写接口 Mock 测试
    - [ ] 设计支持多 API 切换的 RHI 句柄与资源抽象
- [ ] Task: 实现 OpenGL 4.6 RHI 后端
    - [ ] [TDD] 编写基础绘制指令测试
    - [ ] 实现 OpenGL 下的缓冲区、管线状态与纹理创建
- [ ] Task: 实现基础场景树 (SceneTree) 与 TRS 变换
    - [ ] [TDD] 编写父子节点矩阵计算测试
    - [ ] 实现 `FSceneNode` 与坐标空间转换逻辑
- [ ] Task: Conductor - 用户手册验证 'Phase 3: RHI 抽象层 (OpenGL) 与场景树' (Protocol in workflow.md)

## Phase 4: 模块化 Shader 系统与 SPIR-V 编译

- [ ] Task: 集成 glslang 与 SPIR-V 工具链
    - [ ] [TDD] 编写 Shader 源码编译为 SPIR-V 的测试
    - [ ] 实现 `FShaderCompiler` 包装类
- [ ] Task: 实现模块化 Shader 预处理器 (#include)
    - [ ] [TDD] 编写包含递归解析的测试用例
    - [ ] 实现支持虚拟文件系统的 Shader 预处理逻辑
- [ ] Task: Conductor - 用户手册验证 'Phase 4: 模块化 Shader 系统与 SPIR-V 编译' (Protocol in workflow.md)
