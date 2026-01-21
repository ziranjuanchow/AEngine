# Track 实施计划 - 场景大纲与层级管理

## Phase 1: 场景基础设施增强 [checkpoint: 540f70e]

- [x] Task: 扩展 FSceneNode 功能
    - [x] [TDD] 编写可见性切换与名称修改测试 (tests/SceneTests.cpp)
    - [x] 在 `FSceneNode` 中添加 `m_isVisible` 标志和 `m_name` 修改接口 (src/Core/SceneNode.h)
- [x] Task: 实现递归渲染数据收集
    - [x] [TDD] 验证递归遍历能否正确收集所有子节点的 FRenderable (逻辑简单，直接集成验证)
    - [x] 在 `SandboxApp` 中实现 `CollectRenderables(FSceneNode* node, ...)` 逻辑，替换目前的扁平列表
- [x] Task: Conductor - User Manual Verification 'Phase 1: 场景基础设施增强' (Protocol in workflow.md)

## Phase 2: 场景大纲面板 (Scene Outliner) [checkpoint: 624c2ee]

- [x] Task: 实现树状大纲 UI
    - [x] 编写递归函数使用 `ImGui::TreeNode` 显示场景结构 (main.cpp)
    - [x] 实现基础的单选与多选逻辑 (Selection State)
- [x] Task: 增加节点操作功能
    - [x] 实现右键菜单：删除选中节点、重命名节点 (重命名放在属性面板)
    - [x] 实现可见性开关（眼睛图标）的 UI 绑定 (Checkbox)
- [x] Task: Conductor - User Manual Verification 'Phase 2: 场景大纲面板' (Protocol in workflow.md)

## Phase 3: 属性检查器面板 (Inspector)

- [x] Task: 实现变换编辑器 (Transform Editor)
    - [x] 为选中节点提供 Position, Rotation, Scale 的 DragFloat 输入框 (main.cpp)
    - [x] 验证父节点变换修改后，子节点矩阵自动同步更新 (FSceneNode 已支持)
- [x] Task: 实现材质编辑器 (Material Editor)
    - [x] 自动识别选中节点是否包含网格/材质
    - [x] 提供 PBR 参数（Albedo, Roughness 等）的实时滑块
- [ ] Task: Conductor - User Manual Verification 'Phase 3: 属性检查器面板' (Protocol in workflow.md)

## Phase 4: 追加式加载与最终集成

- [ ] Task: 优化模型加载流
    - [ ] 修改 `SandboxApp::OnImGuiRender`，使新加载的模型挂载到当前选中节点或 Root 下
- [ ] Task: 全面回归验证
    - [ ] 验证多模型复杂场景下的渲染与编辑稳定性
- [ ] Task: Conductor - User Manual Verification 'Phase 4: 追加式加载与最终集成' (Protocol in workflow.md)
