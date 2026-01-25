# Track 实施计划 - 后期处理与 HDR 管线

## Phase 1: HDR 基础设施与 ACES Tone Mapping

- [x] Task: 升级渲染目标为 HDR
    - [x] [TDD] 验证 HDR 纹理 (RGBA16F) 的创建与采样 (tests/RHITests.cpp)
    - [x] 修改 `main.cpp` 中的主渲染目标（用于替代屏幕默认 Framebuffer 的 Lighting 目标）
- [x] Task: 实现基础后期处理 Pass
    - [x] 编写 `shaders/PostProcess.vert` (全屏三角形)
    - [x] 编写 `shaders/ACES_ToneMapping.frag` (含 Exposure 和 Gamma 校正)
    - [x] 实现 `FPostProcessPass`：渲染 HDR 纹理到屏幕 (src/RHI/PostProcessPass.h/cpp)
- [ ] Task: Conductor - User Manual Verification 'Phase 1: HDR 基础设施' (Protocol in workflow.md)

## Phase 2: Bloom 效果实现

- [ ] Task: 实现亮部提取与降采样
    - [ ] 编写 `shaders/BloomExtract.frag`
    - [ ] 实现一系列临时 FBO 进行 Dual Filtering 或 Gaussian Blur
- [ ] Task: 实现 Bloom 叠加
    - [ ] 编写 `shaders/BloomCombine.frag` 将模糊后的亮部与原图混合
- [ ] Task: Conductor - User Manual Verification 'Phase 2: Bloom 效果' (Protocol in workflow.md)

## Phase 3: 后期处理 UI 与集成

- [ ] Task: 实现 Post-processing 面板
    - [ ] 添加 Exposure, Bloom Intensity, Vignette Power 的 ImGui 控件
    - [ ] 支持在 UI 中动态开关各个后期处理子项
- [ ] Task: 最终集成与调优
    - [ ] 调整点光源强度，验证 HDR 的动态范围表现
- [ ] Task: Conductor - User Manual Verification 'Phase 3: 后期处理 UI' (Protocol in workflow.md)
