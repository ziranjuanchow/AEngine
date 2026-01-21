# 功能说明：场景编辑器 (Scene Editor)

## 1. 简介
AEngine 提供了基于 ImGui 的场景编辑器界面，允许用户查看、组织和修改场景中的物体。

## 2. 核心功能

### 2.1 场景大纲 (Scene Hierarchy)
- **树状显示**：递归显示 `FSceneNode` 的父子关系。
- **节点操作**：
    - **选中**：单击节点以选中。
    - **删除**：右键菜单选择 "Delete"。
    - **可见性**：点击眼睛图标（Checkbox）切换 `IsVisible`。

### 2.2 属性检查器 (Inspector)
选中节点后，Inspector 面板会显示：
- **变换 (Transform)**：实时编辑位置、旋转、缩放。
- **材质 (Material)**：
    - 自动识别并列出节点关联的所有材质。
    - 支持调节 PBR 参数（Albedo, Metallic, Roughness）。
    - 支持更换贴图（目前通过代码逻辑，UI 暂未暴露文件选择）。

### 2.3 交互逻辑
- **模型加载**：新导入的模型（通过 Asset Loader）会自动挂载到当前选中节点下，或者挂载到 Root 节点。
- **相机控制**：WASD + 右键旋转。提供 "Reset Camera" 按钮。

## 3. 技术实现
- **递归渲染**：`SandboxApp` 使用 `CollectRenderables` 从根节点递归收集绘制数据。
- **延迟操作**：为了安全地在遍历中删除节点，使用了 `m_deferredActions` 队列。
