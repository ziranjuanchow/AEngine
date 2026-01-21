# Track 规格说明 - 阴影映射 (Shadow Mapping)

## 1. 概述
本 Track 旨在为 AEngine 引入实时阴影渲染能力。通过在渲染管线中集成专门的阴影生成通道（Shadow Pass），支持方向光产生高质量的实时阴影，并提供多种技术方案的可切换支持。

## 2. 功能需求
### 2.1 阴影图生成 (Shadow Pass)
- 在 `FRenderGraph` 中实现 `FShadowPass`。
- 负责将场景物体（不透明物）渲染到深度缓冲区（Depth Texture）。
- **Shader 实现**：编写专用的 `ShadowDepth` 着色器，仅输出深度值。

### 2.2 阴影技术方案 (多级可选)
在 ImGui 编辑器中提供下拉框，支持以下方案切换：
- **Hard Shadows**：标准深度对比。
- **PCF (Percentage Closer Filtering)**：实现 3x3 或 5x5 的邻域平均采样，实现软阴影边缘。
- **PCSS (Percentage Closer Soft Shadows)** (预留/进阶)：根据遮挡物距离动态计算半影宽度（如果时间允许，先打好框架）。

### 2.3 渲染管线集成
- **数据流**：`FShadowPass` 生成的 Depth Texture 将作为 `FForwardLitPass` 的输入纹理。
- **光照计算**：更新 `StandardPBR.frag`，引入阴影项（Shadow Term）的计算。
- **空间变换**：计算并传递 Light Space Matrix（光源空间的 View-Projection 矩阵）。

## 3. UI 交互 (ImGui)
- 增加 "Shadow Settings" 面板：
    - 下拉框选择阴影技术方案。
    - 调节 Shadow Bias 以解决 Shadow Acne（阴影粉刺）问题。
    - 调节阴影贴图分辨率。

## 4. 技术约束
- 阴影贴图应支持可配置的分辨率（默认 2048x2048）。
- 必须使用 RHI 接口进行资源创建和命令提交。

## 5. 验收标准
- 场景中能够看到明确的阴影投射（如球体投射到地面）。
- 切换不同方案时，阴影边缘质量有明显视觉变化。
- 调整光源方向时，阴影能够实时且正确地更新位置。

## 6. 超出范围
- 全向光源阴影 (Point Light Shadows)。
- 透明物体阴影。
- 动态物体与静态物体的阴影合并。
