# RHI 抽象层重构 - 规范 (Specification)

## 1. 目标 (Goal)
通过将渲染管线中遗留的原始 OpenGL 调用（特别是 `glDrawBuffers`, `glBlendFunc`, `glBlitFramebuffer`）抽象到 RHI（渲染硬件接口）层，提升渲染器架构的解耦性、模块化和可移植性，为未来支持多图形 API (如 Vulkan 或 DirectX 12) 奠定基础。

## 2. 动机 (Motivation)
当前的渲染管线实现中，核心渲染逻辑与 OpenGL API 之间存在直接调用，这导致 `RenderModule` 与特定的图形 API 紧密耦合。这种紧耦合阻碍了代码的可维护性、自动化测试的实现，并增加了未来扩展到其他渲染 API 的难度。通过将这些 API 调用封装到 RHI 层，可以实现更清晰的职责分离，提升整体架构的健壮性。

## 3. 范围 (Scope)
*   将 `glDrawBuffers` 的功能抽象到 `IRHIDevice` 或 `IRHICommandBuffer` 接口中。
*   将 `glBlendFunc` 的功能抽象到 `IRHIDevice` 或 `IRHICommandBuffer` 接口中，提供统一的混合模式管理。
*   将 `glBlitFramebuffer` 的功能抽象到 `IRHIDevice` 或 `IRHICommandBuffer` 接口中，用于帧缓冲区之间的复制操作。
*   修改所有受影响的渲染器内部逻辑（如 `FSceneRenderer`, `FOpenGLDevice`, `OpenGLCommandBuffer`），使其通过新的 RHI 接口进行上述操作。

## 4. 范围外 (Out of Scope)
*   **不包括**立即实现完整的 Vulkan/DirectX 12 后端。本次重构仅限于抽象接口，为多 API 支持做准备。
*   **不包括**所有 OpenGL API 调用的全面重构。本次只专注于已识别的关键功能。
*   **不包括**引入新的第三方 RHI 抽象库。我们将继续维护内部的 RHI 抽象层。

## 5. 成功标准 (Success Criteria)
*   在核心渲染逻辑（如 `FSceneRenderer`）中，不再存在直接的 `glDrawBuffers`、`glBlendFunc`、`glBlitFramebuffer` 调用。
*   所有这些操作都通过统一的 `IRHIDevice` 或 `IRHICommandBuffer` 接口进行。
*   项目能够成功编译和运行，功能与重构前保持一致。
*   所有现有的单元测试和集成测试通过。

## 6. 技术决策 (Technical Decisions)

### 6.1 采用的解决方案
**选择的方案：** 扩展现有的 `IRHIDevice` 接口，并引入或增强 `IRHICommandBuffer` 接口，以封装 `glDrawBuffers`、`glBlendFunc` 和 `glBlitFramebuffer` 等 OpenGL 功能。具体的 OpenGL 实现将仅限于 `FOpenGLDevice` 和 `FOpenGLCommandBuffer` 类。

**理由：**
*   **职责分离：** 通过将特定于 OpenGL 的调用移动到 RHI 实现层，可以确保渲染器上层逻辑的 API 不可知性。
*   **可扩展性：** 为未来添加 Vulkan 或 DirectX 12 后端提供了清晰的扩展点，只需实现新的 `IRHIDevice` 和 `IRHICommandBuffer` 派生类即可。
*   **可维护性：** 集中管理图形 API 相关操作，降低了维护成本和引入错误的风险。
*   **对齐项目风格：** 延续了项目已有的 RHI 抽象模式和 UE-Style 命名约定。

### 6.2 考虑但未采用的替代方案

*   **替代方案 1：立即实现完整的 Vulkan/DirectX 12 后端。**
    *   **拒绝理由：** 该方案的工作量巨大，超出了本次增量架构改进的范畴，且会大幅延长开发周期。目前的重点是解耦现有架构，而不是立即进行全面的 API 迁移。

*   **替代方案 2：引入第三方 RHI 抽象库。**
    *   **拒绝理由：** 虽然第三方库可以提供现成的解决方案，但这会引入额外的外部依赖，可能与 AEngine 现有的设计哲学（如 C++20 标准、UE-Style 命名、对核心引擎行为的完全控制）不符。AEngine 倾向于维护一个轻量级、高度可控的内部抽象层。
