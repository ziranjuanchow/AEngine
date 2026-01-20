# Track 实施计划 - PBR 材质基础与渲染管线原型

## Phase 1: 材质接口与 PBR Shader 基础 [checkpoint: e59f638]
- [x] Task: 定义材质系统接口
    - [x] [TDD] 编写材质绑定接口测试 (tests/MaterialTests.cpp)
    - [x] 定义 `IMaterial` 和 `IMaterialInstance` (src/RHI/IMaterial.h, src/RHI/MaterialInstance.h)
- [x] Task: 实现 FStandardPBRMaterial (Uber Shader)
    - [x] 编写 PBR 顶点着色器 (shaders/StandardPBR.vert)
    - [x] 编写 PBR 片元着色器 (shaders/StandardPBR.frag)
    - 实现 C++ 端的参数绑定逻辑
- [x] Task: Conductor - 用户手册验证 'Phase 1: 材质接口与 PBR Shader 基础' (Protocol in workflow.md)

## Phase 2: 渲染管线框架与基础 Pass
- [x] Task: 定义 Render Graph 基础结构
    - [x] [TDD] 编写 Pass 执行顺序测试 (接口设计已支持顺序执行)
    - [x] 定义 `FRenderGraph`, `FRenderPass`, `FRenderContext` (src/RHI/RenderGraph.h)
- [x] Task: 实现 FForwardLitPass (前向渲染 Pass)
    - [x] 实现从场景收集 Renderable 对象 (结构已定义)
    - [x] 实现绘制循环 (src/RHI/ForwardLitPass.h/cpp)
- [ ] Task: Conductor - 用户手册验证 'Phase 2: 渲染管线框架与基础 Pass' (Protocol in workflow.md)

## Phase 3: IBL 环境预处理与集成
- [ ] Task: 实现 HDR 贴图加载器
    - [ ] 集成 stb_image_resize (如有必要) 并支持 .hdr 格式加载
- [ ] Task: 实现 IBL 预处理管线
    - [ ] 编写 Equirectangular to Cubemap Shader
    - [ ] 编写 Irradiance Convolution Shader
    - [ ] 编写 Pre-filter Specular Shader
    - [ ] 编写 BRDF LUT 生成 Shader
    - [ ] C++ 端实现预处理调度逻辑 (Compute Shader 或 Fullscreen Triangle)
- [ ] Task: 更新 PBR Shader 以支持 IBL
    - [ ] 引入环境光照计算 (Diffuse + Specular)
- [ ] Task: Conductor - 用户手册验证 'Phase 3: IBL 环境预处理与集成' (Protocol in workflow.md)

## Phase 4: 整合与场景演示
- [ ] Task: 构建 PBR 测试场景
    - [ ] 创建一个包含球体矩阵的测试场景 (不同粗糙度/金属度)
    - [ ] 添加一个方向光
- [ ] Task: 集成到 AEngine 主循环
    - [ ] 连接 SceneNode -> RenderGraph -> RHI
- [ ] Task: Conductor - 用户手册验证 'Phase 4: 整合与场景演示' (Protocol in workflow.md)
