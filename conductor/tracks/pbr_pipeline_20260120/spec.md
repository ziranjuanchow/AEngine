# Track 规格说明 - PBR 材质基础与渲染管线原型

## 1. 概述
本 Track 旨在为 AEngine 引入基于物理的渲染 (PBR) 能力，并搭建起第一个可扩展的渲染管线框架。通过实现 IBL (Image Based Lighting) 和直接光照，确保渲染出的物体具有物理正确的外观。

## 2. 功能需求
### 2.1 PBR 材质系统
- 定义抽象 `IMaterial` 接口，支持绑定 Pipeline State 和资源。
- 实现 `FStandardPBRMaterial` (作为 MVP 的默认实现)，基于 Cook-Torrance BRDF Uber Shader。
- 材质参数包括：Albedo, Normal, Metallic, Roughness, AO。
- **架构目标**：确保未来可以无缝引入 Shader Graph 生成的材质，而不破坏渲染管线。

### 2.2 基础渲染管线 (Render Graph 原型)
- 实现简单的 `FRenderPass` 抽象类。
- 实现 `FForwardLitPass`：负责绘制场景中的不透明物体并应用 PBR 光照。
- 实现渲染器核心逻辑：从场景树中提取渲染数据 (Draw Calls) 并分配给对应的 Pass。

### 2.3 IBL 与环境预处理
- 支持加载 HDR 全景环境贴图。
- 实现 GPU 实时预处理逻辑：
    - 将 HDR 转为 Cubemap。
    - 计算 Diffuse Irradiance Map。
    - 计算 Specular Pre-filtered Map。
    - 计算 BRDF LUT (Look-up Texture)。

### 2.4 直接光照
- 支持至少一种光源类型（如方向光 `FDirectionalLight`）。

## 3. 非功能需求
- 性能：实时预处理过程应在 1-2 秒内完成。
- 模块化：渲染 Pass 应易于添加和删除。

## 4. 验收标准
- 能够在窗口中显示一个具有 PBR 材质的几何体（如球体）。
- 几何体表面能正确反射环境贴图的内容。
- 调整粗糙度和金属度参数时，视觉效果发生符合物理规律的变化。
- 控制台无报错，每帧保持稳定刷新。

## 5. 超出范围
- 延迟渲染管线 (Deferred Rendering)。
- 阴影映射 (Shadow Mapping)。
- 动态光源管理。
