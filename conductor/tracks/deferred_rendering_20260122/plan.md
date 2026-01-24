# Track 实施计划 - 延迟渲染管线

## Phase 1: G-Buffer 基础设施 [checkpoint: 0951e7e]

- [x] Task: 扩展 RHI 支持 MRT (多渲染目标)
    - [x] [TDD] 验证 Framebuffer 创建时附带多个 Color Attachments (tests/RHITests.cpp)
    - [x] 更新 `FFramebufferConfig` 结构以支持 `std::vector<shared_ptr<IRHITexture>> ColorAttachments`
    - [x] 在 `FOpenGLFramebuffer` 中实现 `glDrawBuffers` 配置
- [x] Task: 实现 G-Buffer Pass
    - [x] 编写 `shaders/DeferredGeometry.vert/frag` (输出到 Location 0, 1, 2)
    - [x] 实现 `FDeferredGeometryPass`：配置 MRT Framebuffer 并渲染场景 (src/RHI/DeferredGeometryPass.h/cpp)
- [x] Task: Conductor - User Manual Verification 'Phase 1: G-Buffer 基础设施' (Protocol in workflow.md)

## Phase 2: 延迟光照与光体积

- [x] Task: 实现光体积渲染逻辑
    - [x] 扩展 `FGeometryUtils` 或 `SandboxApp`，生成大量点光源数据（位置、颜色、半径） (main.cpp)
    - [x] 在 RHI 中启用 Additive Blending (`glBlendFunc`) (src/RHI/IRHIDevice.h, OpenGLCommandBuffer.cpp)
- [x] Task: 实现光照 Pass
    - [x] 编写 `shaders/DeferredLighting.vert/frag` (从 G-Buffer 采样并计算 PBR) (shaders/DeferredLighting.vert/frag)
    - [x] 实现 `FDeferredLightingPass`：绑定 G-Buffer 纹理，绘制光球 (src/RHI/DeferredLightingPass.h/cpp)
- [x] Task: Conductor - User Manual Verification 'Phase 2: 延迟光照与光体积' (Protocol in workflow.md)

## Phase 3: 混合管线与最终集成 [checkpoint: pending]

- [x] Task: 实现深度缓冲区 Blit (复制)
    - [x] 在 `IRHIDevice` 中添加 `BlitFramebuffer` 接口 (src/RHI/IRHIDevice.h)
    - [x] 在 `FOpenGLDevice` 中实现 `glBlitFramebuffer` (src/RHI/OpenGL/OpenGLDevice.cpp)
- [x] Task: 集成 Forward Pass (处理天空盒与调试图形)
    - [x] 调整 `FRenderGraph` 顺序：Geometry -> Lighting -> Forward (main.cpp)
    - [x] 确保 Forward Pass 复用已填充的深度缓冲进行深度测试 (通过 BlitFramebuffer 实现)
- [x] Task: Conductor - User Manual Verification 'Phase 3: 混合管线与最终集成' (Protocol in workflow.md)
