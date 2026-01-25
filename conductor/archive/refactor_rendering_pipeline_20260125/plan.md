# Track 实施计划 - 渲染管线重构与修复

## Phase 1: 基础设施重构 (SceneRenderer)

- [x] Task: 创建 `FSceneRenderer` 类
    - [x] 将 `main.cpp` 中的 `m_renderGraph`, `m_device`, `m_gBuffer`, `m_hdrFBO` 等移入 `FSceneRenderer`。 (src/RHI/SceneRenderer.h/cpp)
    - [x] 实现 `FSceneRenderer::Init` 和 `FSceneRenderer::Resize`。
- [x] Task: 迁移 Pass 初始化逻辑
    - [x] 将 `ShadowPass`, `DeferredGeometryPass`, `DeferredLightingPass`, `ForwardLitPass`, `PostProcessPass` 的创建移入 `SceneRenderer`。
- [x] Task: 实现 `FSceneRenderer::Render`
    - [x] 封装原本在 `main.cpp::OnUpdate` 中的渲染循环。
    - [x] 确保 Viewport 在每个 Pass 开始前根据 FBO 尺寸自动设置。

## Phase 2: 深度与纹理状态修复

- [x] Task: 修复 G-Buffer 深度冲突
    - [x] 验证并固化 `LightingFBO` (无深度) 和 `ForwardFBO` (共享深度) 的设计。 (src/RHI/SceneRenderer.cpp)
    - [x] 确保在 Pass 切换时显式解绑纹理（防止 Feedback Loop）。
- [x] Task: 修复 Resize 问题
    - [x] 在 `FSceneRenderer::Resize` 中重建所有 FBO。
    - [x] 验证窗口拉伸后，光照和几何体位置是否对齐。 (src/RHI/SceneRenderer.cpp, DeferredLightingPass.h)

## Phase 3: 验证与清理

- [x] Task: 清理 `main.cpp`
    - [x] 移除所有残留的渲染代码，只保留 `App` 逻辑和 UI 逻辑。 (main.cpp)
- [x] Task: Conductor - User Manual Verification 'Phase 3: 验证与清理' (Protocol in workflow.md)
