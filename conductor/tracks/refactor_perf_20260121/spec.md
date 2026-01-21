# Track 规格说明 - 性能优化与代码清理 (Performance & Refactor)

## 1. 概述
本 Track 旨在提升 AEngine 的代码质量、可维护性和运行时效率。通过重构主程序结构、引入状态缓存机制以及完善 API 注释，为后续更高级的功能开发夯实基础。

## 2. 功能需求
### 2.1 架构重构 (Refactoring)
- **Application 类封装**：
    - 定义 `FApplication` 基类，封装 `OnInit`, `OnUpdate`, `OnShutdown` 和 `Run` 逻辑。
    - 清理 `main.cpp`，将其职责简化为应用程序的入口点。
- **注释完善**：
    - 为所有 `src/Core` 和 `src/RHI` 下的核心接口添加 Doxygen 兼容的注释（包括参数、返回值和设计初衷）。

### 2.2 性能优化 (Optimization)
- **Shader 编译缓存 (Memory Cache)**：
    - 在 `FShaderCompiler` 中引入基于源码哈希的内存缓存，避免同一帧或频繁重复编译。
- **RHI 状态缓存 (State Tracking)**：
    - 在 `FOpenGLCommandBuffer` 中实现基础的状态追踪，避免重复绑定相同的 VAO、VBO 或着色器程序。
- **内存优化**：
    - 审查 `FSceneNode` 的矩阵更新频率，减少不必要的计算。

## 3. 技术约束
- 保持现有的 UE 风格命名习惯。
- 优化不应破坏现有的 PBR 渲染效果和 Assimp 加载能力。

## 4. 验收标准
- `main.cpp` 的代码量减少 70% 以上。
- 能够通过继承 `FApplication` 快速创建新的测试 Demo。
- 通过控制台日志确认 Shader 编译在重复请求时被跳过（命中缓存）。
- 渲染效果（红色球体、模型加载）与重构前保持一致。

## 5. 出门范围
- 磁盘级的 Shader 编译缓存（本 Track 仅实现内存级）。
- 复杂的多线程内存池。
