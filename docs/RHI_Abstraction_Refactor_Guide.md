# RHI 抽象去 OpenGL 常量：意义与分步说明

## 为什么要做这件事

这次改造的核心目标，是让 RHI 从“形式上的抽象”变成“真正可移植的抽象”。当前接口中仍存在 `uint32_t` + OpenGL 常量语义（例如深度比较与剔除模式），会带来以下问题：

- 抽象泄漏：上层逻辑隐式依赖 OpenGL 语义，RHI 分层被削弱。
- 可移植性差：未来接入 Vulkan / DX12 / Metal 时，需要额外清理调用层。
- 类型不安全：`uint32_t` 可传任意值，错误很难在编译期暴露。
- 可读性差：魔法数字不直观，维护成本高。

通过引入引擎自定义枚举，并把图形 API 映射下沉到后端，可显著提升可维护性、扩展性与稳定性。

## 每一步的意义

### 1. 在 `RHIDefinitions.h` 定义统一枚举
新增 `ERHICompareFunc`、`ERHICullMode`（可选：重定义 `ERHIBlitMask` 为纯引擎位标志）。

意义：建立跨后端共享的“渲染语义字典”，使上层表达与底层 API 解耦。

### 2. 修改 `IRHIDevice.h` / `IRHICommandBuffer` 接口签名
将 `SetDepthTest(..., uint32_t func)` 改为 `SetDepthTest(..., ERHICompareFunc func)`，将 `SetCullMode(uint32_t)` 改为 `SetCullMode(ERHICullMode)`。

意义：把类型安全前置到编译期，减少运行期隐性错误。

### 3. 在 OpenGL 后端实现枚举到 GL 的映射
在 `OpenGLCommandBuffer` 中新增 `ConvertCompareFunc`、`ConvertCullMode`。

意义：把平台差异收敛在 backend，遵循“上层语义统一、下层各自实现”的架构原则。

### 4. 全局替换调用点
将所有调用 `SetDepthTest` / `SetCullMode` 的地方改为新枚举。

意义：避免新旧接口并存造成维护混乱，保证代码风格与语义一致。

### 5. 补接口层回归测试
在 `tests/RHITests.cpp` 增加 mock 测试，验证新签名与参数传递正确。

意义：确保本次改动是“安全重构”，而不是功能变化。

### 6. 构建与测试验证
执行 `./build.ps1 -NoTest`、`./build.ps1` 或 `ctest`。

意义：确认改造可集成、无回归，降低后续迭代风险。

## 预期收益

- RHI 边界更清晰，OpenGL 不再向上层泄漏。
- 多后端演进成本显著降低。
- 接口可读性、稳定性、可测试性全面提升。
