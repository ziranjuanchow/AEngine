# Track 规格说明 - 延迟渲染管线 (Deferred Rendering)

## 1. 概述
本 Track 旨在为 AEngine 引入延迟渲染 (Deferred Rendering) 架构。通过将几何属性（G-Buffer）与光照计算分离，支持在场景中高效渲染大量动态光源，并保持 PBR 材质的一致性。

## 2. 技术选型与决策 (Technical Decisions)

### 2.1 G-Buffer 布局
- **选中方案**: 标准配置 (Classic G-Buffer)。
    - *理由*: 兼容性好，无需法线压缩，适合 MVP。
- **替代方案 (已拒绝)**:
    - *Compact G-Buffer*: 虽然带宽更低，但在 MVP 阶段增加了法线编解码的复杂性，且对 Shader 算力有额外要求。

### 2.2 光照处理方式
- **选中方案**: 光体积 (Light Volumes)。
    - *理由*: 能有效剔除不受光源影响的像素，适合大量局部点光源场景。
- **替代方案 (已拒绝)**:
    - *Fullscreen Quad*: 实现简单但 Overdraw 严重，无法发挥延迟渲染处理多光源的优势。
    - *Tile-based Deferred*: 实现复杂度过高，暂不适合当前阶段。

### 2.3 半透明处理
- **选中方案**: Forward Pass 回退。
    - *理由*: 标准且稳健的解决方案。
- **替代方案 (已拒绝)**:
    - *Weighted Blended OIT*: 算法复杂，对显存带宽要求高，且不是延迟渲染的核心部分。

## 3. 功能需求
### 3.1 G-Buffer 生成 (Geometry Pass)
- 扩展 RHI 以支持 MRT (Multiple Render Targets)。
- 创建 `FDeferredGeometryPass`。
- G-Buffer 布局：
    - RT0 (RGBA8): Albedo.rgb, Roughness.a
    - RT1 (RGBA16F): Normal.rgb, Metallic.a
    - RT2 (RGBA8): Emissive.rgb, AO.a
    - Depth (D24S8)

### 3.2 延迟光照 (Lighting Pass)
- 创建 `FDeferredLightingPass`。
- 实现光源体积渲染：
    - 使用 `FGeometryUtils::CreateSphere` 作为光照体积。
    - 启用 Additive Blending (glBlendFunc(GL_ONE, GL_ONE))。
    - 实现 Light Volume 着色器，从 G-Buffer 重建世界坐标和材质属性。

### 3.3 混合管线整合
- 在 `FRenderGraph` 中串联 GeometryPass -> LightingPass -> ForwardLitPass (用于调试或半透明)。
- 实现深度缓冲区拷贝 (BlitFramebuffer)，使得 Forward Pass 可以复用 Geometry Pass 的深度信息。

## 4. 验收标准
- 场景中能够渲染至少 100 个动态点光源而保持帧率稳定。
- G-Buffer 的各个通道（Albedo, Normal, etc.）能在 ImGui 窗口中预览。
- 延迟渲染出的 PBR 效果与之前的 Forward 渲染视觉一致。
