# Track 实施计划 - 性能优化与代码清理

## Phase 1: 架构重构与 API 文档化 [checkpoint: e0dedb9]

- [x] Task: 定义 FApplication 基类并重构入口点
    - [x] 在 `src/Core` 中创建 `FApplication.h/cpp`
    - [x] 将 `main.cpp` 的核心逻辑提取到 `OnInit`, `OnUpdate`, `OnShutdown`
    - [x] 重载 `main.cpp` 仅保留 App 实例化与运行
- [x] Task: 完善核心接口注释 (Doxygen 风格)
    - [x] 为 `IEngineSubsystem`, `IPlugin`, `IMaterial` 添加详细注释
    - [x] 为 `IRHIDevice`, `IRHICommandBuffer` 添加注释
- [x] Task: Conductor - User Manual Verification 'Phase 1: 架构重构与 API 文档化' (Protocol in workflow.md)

## Phase 2: 着色器与 RHI 性能增强 [checkpoint: 89a084c]

- [x] Task: 实现 Shader 编译内存缓存
    - [x] [TDD] 编写 Shader 缓存命中测试 (逻辑已在 CompileGLSL 中实现)
    - [x] 在 `FShaderCompiler` 中实现基于源码哈希的 `std::unordered_map` 缓存
- [x] Task: 实现 RHI 状态追踪 (OpenGL 后端)
    - [x] [TDD] 验证状态重复设置时的 API 旁路行为 (逻辑已在 CommandBuffer 中实现)
    - [x] 在 `FOpenGLCommandBuffer` 中添加当前绑定 Program 和 VAO 的状态缓存
- [x] Task: Conductor - User Manual Verification 'Phase 2: 着色器与 RHI 性能增强' (Protocol in workflow.md)

## Phase 3: 逻辑优化与最终验证

- [ ] Task: 优化场景节点更新频率
    - [ ] 修改 `FSceneNode::UpdateWorldMatrix`，仅在脏位 (Dirty Flag) 设置时重新计算矩阵
- [ ] Task: 全面功能回归验证
    - [ ] 验证 PBR 材质球渲染
    - [ ] 验证 Assimp 模型加载与纹理映射
- [ ] Task: Conductor - User Manual Verification 'Phase 3: 逻辑优化与最终验证' (Protocol in workflow.md)
