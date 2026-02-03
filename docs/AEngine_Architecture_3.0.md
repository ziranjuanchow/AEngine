# AEngine 3.0 架构设计白皮书

## 1. 架构愿景 (Vision)

AEngine 3.0 旨在转型为一个**模块化、微内核驱动的现代游戏引擎**。
核心设计哲学如下：
*   **微内核 (Microkernel)**: 核心层极度轻量，只负责管理模块、生命周期和基础服务。所有功能（包括渲染、窗口）皆为模块。
*   **分层架构 (Layered)**: 严格的依赖层级，禁止反向依赖。
*   **按需加载 (On-Demand)**: 仅加载项目显式启用或被依赖的模块，杜绝资源浪费。
*   **松耦合通信 (Decoupled Communication)**: 结合接口服务与事件代理系统，灵活应对模块间交互。

---

## 2. 系统分层 (System Architecture Layers)

系统自下而上分为五个层级。上层可依赖下层，下层严禁依赖上层。

### Level 0: 核心层 (Kernel Layer)
*   **定位**: 引擎基石，零业务逻辑。
*   **内容**: 
    *   `AEngine.Kernel`: 模块加载器、依赖解析器、配置读取。
    *   `AEngine.Core`: 日志、断言、内存管理、文件系统 (VFS)、**代理系统 (Delegate System)**、RTTI。
*   **形式**: 静态库 (Static Lib) 或 核心动态库 (Shared Lib)。

### Level 1: 引擎模块层 (Engine Modules)
*   **定位**: 构成引擎功能的标准积木。
*   **内容**: 
    *   `Engine.Window`: 窗口管理 (GLFW)。
    *   `Engine.RHI`: 渲染硬件接口定义。
    *   `Engine.Scene`: 场景图数据结构。
    *   `Engine.Renderer`: 标准渲染管线。
*   **形式**: 动态链接库 (DLLs)。

### Level 2: 引擎插件层 (Engine Plugins)
*   **定位**: 接口的具体实现或扩展功能。
*   **内容**:
    *   `Plugin.RHI.OpenGL`: OpenGL 后端实现。
    *   `Plugin.Audio.FMOD`: FMOD 音频集成。
*   **形式**: 动态链接库 (DLLs)，运行时动态加载。

### Level 3: 项目模块层 (Project Modules)
*   **定位**: 用户的游戏逻辑与业务代码。
*   **内容**: `Game.Core`, `Game.Gameplay`。
*   **形式**: 动态链接库 (DLLs)。

### Level 4: 项目插件层 (Project Plugins)
*   **定位**: 项目特有的复用组件。

---

## 3. 核心机制详解 (Core Mechanisms)

### 3.1 模块系统 (Module System)
每个模块必须包含一个描述文件 `module.json`：
```json
{
  "name": "Engine.Renderer",
  "type": "EngineModule",
  "dependencies": ["Engine.Core", "Engine.RHI"]
}
```

**IModule 接口**:
所有模块继承自 `IModule`：
*   `OnLoad()`: DLL 加载时调用，注册反射类型。
*   `OnStartup()`: 引擎初始化阶段，分配资源。
*   `OnShutdown()`: 引擎关闭阶段。
*   `OnUnload()`: 内存释放。

### 3.2 按需加载策略 (On-Demand Loading)
引擎启动流程如下：
1.  **扫描 (Discovery)**: 扫描所有目录，建立模块索引。
2.  **配置读取**: 读取 `Project.config`，获取 `EnabledModules` 列表。
3.  **依赖解析**: 从 Enabled 列表出发，递归查找 `dependencies`，构建 **活动模块图 (Active Module Graph)**。
4.  **加载**: 仅加载活动图中的模块。未被引用的模块（如 Dedicated Server 不需要 Audio）将完全不被加载。

### 3.3 通信机制 (Inter-Module Communication)

#### A. 服务定位器 (Service Locator) —— 强类型交互
用于核心功能的直接调用。
*   **注册**: `Core::RegisterService<IRenderer>(this);`
*   **获取**: `auto* r = Core::GetService<IRenderer>();`
*   **特点**: 调用快，但需要包含头文件（编译时依赖）。

#### B. 代理/事件系统 (Delegate System) —— 松耦合交互
用于跨模块通知和解耦。
*   **定义**: `AE_DECLARE_MULTICAST_DELEGATE(WindowResizedEvent, int, int);`
*   **广播**: `WindowModule` 触发 `OnWindowResized.Broadcast(w, h);`
*   **监听**: `RendererModule` 绑定 `OnWindowResized.Add(this, &Renderer::OnResize);`
*   **特点**: 模块间无需知道对方存在，仅依赖事件定义。

---

## 4. 目录结构规划 (Directory Structure)

```text
root/
├── src/
│   ├── Kernel/                 # Level 0: 引擎心脏 (微内核)
│   │   ├── Core/               # 基础库 (Log, Delegate, Math, VFS)
│   │   └── ModuleManager/      # 模块加载与依赖解析
│   │
│   ├── Engine/                 # 引擎层 (提供标准功能)
│   │   ├── Modules/            # 引擎级模块 (Engine.Window, Engine.Renderer等)
│   │   └── Plugins/            # 引擎级插件 (RHI.OpenGL, Audio.FMOD等)
│   │
│   └── Projects/               # 项目层 (用户业务逻辑)
│       ├── Modules/            # 可复用的项目模块 (游戏通用逻辑)
│       ├── Plugins/            # 项目专用插件
│       └── ProjectA/           # 具体项目文件夹 (如 Sandbox)
│           ├── Source/         # 项目业务源码 (Project.ProjectA)
│           ├── Content/        # 资源文件 (Assets)
│           └── Config/         # 项目配置文件 (EnabledModules, Settings)
```

---

## 5. 迁移路线图 (Migration Roadmap)

### Phase 0: 基础设施 (Infrastructure)
*   [ ] 建立新目录结构。
*   [ ] 实现 `Kernel` 基础：`IModule` 接口, `ModuleManager`, `module.json` 解析器。
*   [ ] 实现 `Core` 基础：`Delegate` 系统, `Log` 迁移。

### Phase 1: 核心拆分 (Core Split)
*   [ ] 将 `WindowSubsystem` 迁移为 `Engine.Window` 模块。
*   [ ] 将 `RenderSubsystem` 迁移为 `Engine.Renderer` 模块。
*   [ ] 解决 CMake 链接依赖。

### Phase 2: 插件化 (Pluginization)
*   [ ] 剥离 `FOpenGLDevice` 为独立插件。
*   [ ] 实现基于配置的 RHI 加载。

### Phase 3: 项目层适配 (Project Layer)
*   [ ] 将 `main.cpp` (Sandbox) 改造为独立的 `Project Module`。
*   [ ] 验证完整的按需加载流程。
