# Track 规格说明 - 后期处理与 HDR 管线 (Post-processing & HDR Pipeline)

## 1. 概述
本 Track 旨在为 AEngine 引入高动态范围 (HDR) 渲染能力和模块化的后期处理管线。

## 2. 技术选型与决策 (Technical Decisions)

### 2.1 渲染目标精度
- **选中方案**: RGBA16F (Half Float)。
    - *理由*: 完美的平衡点，提供足够的动态范围支持 HDR，且显存带宽开销可控。
- **替代方案 (已拒绝)**:
    - *RGBA32F*: 显存占用翻倍，对于目前的即时渲染需求来说过剩。
    - *R11G11B10F*: 缺少 Alpha 通道，虽然节省显存，但某些后期效果（如 Bloom）可能需要 Alpha 存储额外信息。

### 2.2 色调映射 (Tone Mapping) 算法
- **选中方案**: ACES (Academy Color Encoding System)。
    - *理由*: 行业标准，提供最佳的对比度和颜色保护，尤其在处理极亮区域时效果显著。
- **替代方案 (已拒绝)**:
    - *Reinhard*: 虽然计算极快，但在高亮区域容易导致色彩饱和度丢失（变灰）。
    - *Filmic (Uncharted 2)*: 效果不错，但参数较多，ACES 是目前更通用的选择。

### 2.3 架构组织
- **选中方案**: 多 Pass 链式执行 (Multi-Pass Chain)。
    - *理由*: 高度模块化，允许用户根据性能需求或视觉偏好动态开启/关闭特定效果。
- **替代方案 (已拒绝)**:
    - *Uber Post-shader*: 将所有逻辑写在一个 Shader 中。虽然减少了 Draw Call，但导致 Shader 极其复杂，难以维护。
    - *Compute Shader*: 对于简单的 Tone Mapping，Pixel Shader 效率足够。

## 3. 功能需求
### 3.1 HDR 基础设施
- 修改主渲染循环，将 Lighting Pass 的输出目标改为 RGBA16F 格式的 HDR Framebuffer。
- 确保所有 PBR 计算在 Linear 空间进行。

### 3.2 后期处理 Pass 套件
- **Tone Mapping Pass**: 实现 ACES 拟合曲线。
- **Exposure Pass**: 实现基于数值的曝光缩放。
- **Bloom Pass**: 
    - 提取亮部 (Thresholding)。
    - 模糊处理 (Gaussian Blur / Downsample & Upsample)。
    - 叠加回主画面。
- **Gamma Correction**: 最终输出前执行 `pow(color, 1.0/2.2)`。
- **Vignette**: 画面边缘渐变压暗。

### 3.3 后期处理 UI (Post Process Volume)
- 在编辑器中添加 "Post-processing" 面板。
- 提供滑块：Exposure (0.1 - 10.0), Bloom Strength (0 - 1.0), Vignette Power (0 - 5.0)。
- 提供每个效果的 Enable/Disable 开关。

## 4. 验收标准
- 画面不再出现 1.0 以上截断导致的白色色块（除非通过 Tone Mapping 模拟）。
- Bloom 效果平滑自然，无明显的采样锯齿。
- 在 UI 中关闭所有后期处理后，画面应能回退到原始 HDR。

## 5. 超出范围
- 自动曝光 (Auto-Exposure)。
- 高级抗锯齿 (TAA/FXAA)。
