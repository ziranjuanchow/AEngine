# Track 规格说明 - 渲染管线重构与修复

## 1. 问题陈述
当前的渲染管线存在严重的架构缺陷和状态管理问题，导致：
- **G-Buffer 深度丢失**：由于 FBO 附件绑定冲突（Feedback Loop）和错误的 Clear 逻辑。
- **Viewport 错位**：`main.cpp` 和 Pass 之间的 Viewport 状态不同步，导致在非默认分辨率下渲染错误。
- **代码不可维护**：`main.cpp` 承担了过多的渲染管线组装和状态管理职责，导致逻辑脆弱且难以扩展。

## 2. 重构目标
### 2.1 引入 `SceneRenderer`
- 创建 `FSceneRenderer` 类，负责管理 `RenderGraph`、FBOs、Passes 和渲染循环。
- `main.cpp` 只负责将 Scene 数据传递给 `SceneRenderer`。

### 2.2 增强 `RenderGraph`
- 支持 Pass 间的显式依赖声明（虽然 MVP 阶段可能还是顺序执行，但接口要清晰）。
- 统一管理 Pass 的 `Execute` 接口，自动处理 Viewport 和 FBO Bind/Unbind。

### 2.3 修复 FBO 与纹理状态
- **严格分离读写**：确保同一纹理不会在同一 Pass 中同时作为 FBO Attachment 和 Shader Sampler（除非只读）。
- **FBO 尺寸管理**：FBO 应响应窗口 Resize 事件，或者拥有独立的 Resolution Scale。

## 3. 技术方案
- **SceneRenderer**: `void Render(const FScene& scene, const FCamera& camera);`
- **Pass Execution**: `pass->Execute(cmdBuffer, renderContext);` -> 内部自动 `SetViewport(0, 0, fboWidth, fboHeight)`.

## 4. 验收标准
- 移除 `main.cpp` 中所有的 `gl*` 调用（除了 ImGui 相关的 backend 初始化）。
- 窗口 Resize 时，G-Buffer 和 HDR FBO 自动重建或 Viewport 正确适配。
- 彻底解决“镜头移动导致画面错误”的 Bug。
