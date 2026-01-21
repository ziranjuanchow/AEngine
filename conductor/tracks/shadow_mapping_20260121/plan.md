# Track 实施计划 - 阴影映射 (Shadow Mapping)

## Phase 1: 阴影基础设施与深度生成 [checkpoint: 019688f]

- [x] Task: 扩展 RHI 支持深度贴图与 FBO
    - [x] [TDD] 验证能否创建仅包含深度槽位的 Framebuffer (已在 OpenGLFramebuffer 实现中验证)
    - [x] 更新 `IRHIDevice` 以支持深度纹理的高级设置（如 Compare Mode）
    - [x] 在 `FOpenGLDevice` 中实现深度缓冲区的创建逻辑 (src/RHI/OpenGL/OpenGLFramebuffer.h/cpp)
- [x] Task: 实现 FShadowPass
    - [x] [TDD] 验证渲染管线中 Shadow Pass 的执行顺序 (通过 RenderGraph 顺序控制)
    - [x] 编写 `shaders/ShadowDepth.vert` 和 `shaders/ShadowDepth.frag` (SPIR-V 编译)
    - [x] 实现 `FShadowPass`：配置深度偏移 (Polygon Offset) 并渲染场景到深度图 (src/RHI/ShadowPass.h/cpp)
- [x] Task: Conductor - User Manual Verification 'Phase 1: 阴影基础设施与深度生成' (Protocol in workflow.md)

## Phase 2: 阴影采样与基础实现 (Hard Shadows)

- [ ] Task: 光源空间变换计算
    - [ ] [TDD] 验证光源投影矩阵 (Orthographic) 的计算正确性
    - [ ] 实现计算 Directional Light View-Projection 矩阵的工具函数
- [ ] Task: 更新 PBR 材质与 Shader
    - [ ] 在 `StandardPBR.frag` 中增加阴影图采样器与 LightSpaceMatrix Uniform
    - [ ] 实现标准深度对比逻辑 (Standard Shadow Mapping)
- [ ] Task: 实现基础 Shadow Bias
    - [ ] 在 UI 中提供 Bias调节，并验证其对“阴影粉刺 (Acne)”的修复效果
- [ ] Task: Conductor - User Manual Verification 'Phase 2: 阴影采样与基础实现 (Hard Shadows)' (Protocol in workflow.md)

## Phase 3: 高级过滤 (PCF) 与多方案切换

- [ ] Task: 实现 PCF 软阴影
    - [ ] 在 Shader 中实现 3x3 和 5x5 的 PCF 过滤内核
    - [ ] 实现基于 Poisson Disk 的随机采样方案（可选增强）
- [ ] Task: 实现阴影设置 UI 与方案切换
    - [ ] 在 ImGui 中添加阴影方案下拉框 (Hard, PCF 3x3, PCF 5x5)
    - [ ] 实现动态更新纹理分辨率的逻辑
- [ ] Task: Conductor - User Manual Verification 'Phase 3: 高级过滤 (PCF) 与多方案切换' (Protocol in workflow.md)

## Phase 4: 回归验证与 Demo 展示

- [ ] Task: 构建阴影测试场景
    - [ ] 创建大型平面作为阴影接收者
    - [ ] 调整光源角度，观察阴影的实时移动与延伸
- [ ] Task: 最终性能检查与代码清理
    - [ ] 确保阴影图在不使用时能正确释放
    - [ ] 补齐新代码的 Doxygen 注释
- [ ] Task: Conductor - User Manual Verification 'Phase 4: 回归验证与 Demo 展示' (Protocol in workflow.md)
