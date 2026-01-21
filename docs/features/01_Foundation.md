# 功能说明：基础架构与环境 (Foundation)

## 1. 简介
本模块构建了 AEngine 的运行时地基，包括启动流程、日志记录、子系统管理、插件加载以及调试 UI 的集成。

## 2. 核心功能

### 2.1 应用程序框架 (Application Framework)
引入了 `FApplication` 类作为引擎的入口点。
- **职责**：管理 `UEngine` 初始化、窗口创建、主循环驱动以及清理。
- **用户接口**：
    - `OnInit()`: 用户自定义初始化逻辑。
    - `OnUpdate(float deltaTime)`: 每一帧的渲染和逻辑更新。
    - `OnImGuiRender()`: 用于绘制调试 UI 和编辑器窗口。

### 2.2 文件选择对话框 (File Dialogs)
集成了 `portable-file-dialogs` 库。
- **功能**：提供原生的操作系统文件选择框，替代了手动输入路径。
- **示例**：
  ```cpp
  auto selection = pfd::open_file("Select a Model", ".", { "3D Models", "*.obj *.fbx" }).result();
  ```

### 2.3 子系统管理 (Subsystem Management)
引擎采用子系统架构来模块化核心功能。
- **注册**：在 `UEngine::Init` 之前通过 `RegisterSubsystem` 注册。
- **生命周期**：
    - `OnStartup()`: 引擎初始化时调用。
    - `OnShutdown()`: 引擎关闭时调用。
- **已实现子系统**：
    - `UWindowSubsystem`: 负责 GLFW 窗口创建、事件轮询和 SwapChain 管理。

### 2.2 插件系统 (Plugin System)
支持动态链接库 (DLL) 的热加载。
- **接口**：所有插件必须实现 `IPlugin` 接口并导出 `CreatePlugin` 函数。
- **上下文同步**：插件加载时会自动调用 `Initialize(void* context)`，同步 ImGui 上下文，确保 UI 在 DLL 中正常渲染。
- **使用方法**：
  ```cpp
  auto& pm = APluginManager::Get();
  pm.LoadPlugin("MyPlugin.dll");
  ```

### 2.3 调试 UI (ImGui Integration)
集成了 **Dear ImGui (Docking 分支)**。
- **特性**：
    - 启用 Docking，允许窗口停靠。
    - 启用 Viewports (多视口)，允许 ImGui 窗口拖出主窗口（目前在某些配置下可能需谨慎开启）。
    - 渲染循环中自动处理 `NewFrame` 和 `Render`，并在 Pass 中通过 `PassthruCentralNode` 确保背景清除正确。

## 3. 开发者指南
- **添加新子系统**：继承 `IEngineSubsystem` 并注册到 `UEngine`。
- **编写插件**：参考 `plugins/StatsPanel` 的实现，注意链接 `imgui` 但不要链接核心引擎库（除非是纯接口）。
