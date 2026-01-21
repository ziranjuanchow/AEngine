# 功能说明：阴影映射 (Shadow Mapping)

## 1. 简介
AEngine 实现了基于深度图的方向光实时阴影。支持基础的硬阴影和基于 PCF (Percentage Closer Filtering) 的软阴影边缘。

## 2. 核心功能

### 2.1 阴影生成 (Shadow Pass)
- **原理**：从光源视角渲染场景到一张 2048x2048 的深度纹理中。
- **RHI 实现**：引入了 `FShadowPass`，它会覆盖物体的原始材质，改用极简的 `ShadowDepth` Shader。
- **抗锯齿/消除粉刺**：支持 `SetDepthBias` (Polygon Offset) 来解决阴影粉刺 (Shadow Acne) 问题。

### 2.2 阴影过滤 (PCF)
- **软阴影**：在 Shader 中实现了 3x3 邻域采样过滤。
- **硬件支持**：利用 `sampler2DShadow` 和 `GL_COMPARE_REF_TO_TEXTURE` 实现了硬件级深度对比。

### 2.3 坐标空间转换
- **LightSpaceMatrix**：自动计算正交投影矩阵，将世界空间位置转换到光源裁剪空间。

## 3. 开发者指南
### UI 设置：
在编辑器中的 "Shadow Settings" 面板可以调节：
- **Bias Constant**: 基础深度偏移。
- **Bias Slope**: 斜率缩放偏移。

## 4. 注意事项
- **分辨率**：默认分辨率为 2048x2048，可在 `main.cpp` 初始化时修改。
- **限制**：目前仅支持单方向光阴影。
