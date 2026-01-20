# Track 实施计划 - Assimp 模型加载

## Phase 1: Assimp 集成与基础解析 [checkpoint: 9f8d956]



- [x] Task: 扩展顶点结构与 RHI 配置


    - [x] [TDD] 验证新顶点结构的对齐 (tests/VertexTests.cpp)
    - [x] 在 `GeometryUtils.h` 中扩展 `FVertex` 以支持切线、副切线和顶点颜色 (移至 RHIDefinitions.h)
    - [x] 更新 `FOpenGLCommandBuffer::SetVertexBuffer` 以支持新的顶点属性布局 (src/RHI/OpenGL/OpenGLCommandBuffer.cpp)
- [x] Task: 实现基础模型加载逻辑
    - [x] 在 `FAssetLoader` 中实现 `LoadModel` 基础骨架 (src/Core/AssetLoader.h/cpp)
    - [x] 实现递归解析 Assimp 节点树并将网格合并/提取
- [x] Task: Conductor - User Manual Verification 'Phase 1: Assimp 集成与基础解析' (Protocol in workflow.md)

## Phase 2: 材质与纹理自动化处理
- [x] Task: 实现纹理加载增强
    - [x] 优化 `FAssetLoader::LoadTexture`，支持自动检测文件是否存在 (src/Core/AssetLoader.h/cpp)
- [x] Task: 实现材质解析逻辑
    - [x] 从 `aiMaterial` 提取贴图路径 (src/Core/AssetLoader.cpp)
    - [x] 自动实例化 `FStandardPBRMaterial` 并绑定加载的纹理
- [ ] Task: Conductor - User Manual Verification 'Phase 2: 材质与纹理自动化处理' (Protocol in workflow.md)

## Phase 3: RHI 转换与场景集成
- [ ] Task: 实现 Assimp Mesh 到 RHI 缓冲区的转换
    - [ ] 处理索引转换和顶点数据打包
    - [ ] 封装为 `std::vector<FRenderable>`
- [ ] Task: 演示场景集成
    - [ ] 更新 `main.cpp`，允许通过 UI 或硬编码加载外部模型
    - [ ] 验证带有法线贴图和基础贴图的模型渲染效果
- [ ] Task: Conductor - User Manual Verification 'Phase 3: RHI 转换与场景集成' (Protocol in workflow.md)
