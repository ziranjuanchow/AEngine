# 功能说明：PBR 渲染管线 (Rendering Pipeline)

## 1. 简介
本模块实现了基于物理的渲染 (PBR) 管线，支持直接光照和基于图像的照明 (IBL)。它建立在自定义的 RHI (Render Hardware Interface) 和 Render Graph 之上。

## 2. 核心功能

### 2.1 材质系统 (Material System)
- **标准 PBR (StandardPBR)**：
    - 采用 Cook-Torrance BRDF 模型。
    - 支持参数：Albedo, Metallic, Roughness, AO。
    - 支持参数绑定：通过 `IMaterial::SetParameter` 动态更新 Uniforms。
- **Shader 编译**：
    - 集成 `glslang`，运行时将 GLSL 源码编译为 SPIR-V 字节码。
    - **技术方案**：针对 OpenGL 4.6 环境，采用 `EShTargetOpenGL_450` 规则编译，允许在 SPIR-V 中使用独立的 Uniforms，从而简化从传统 GLSL 的迁移。
    - 支持 `#include` 预处理指令，方便 Shader 代码复用。

### 2.2 渲染管线 (Render Graph)
- **RenderPass**：将渲染逻辑封装为独立的 Pass。
- **FForwardLitPass**：当前的主渲染 Pass。
    - 负责设置视口和清除状态。
    - 遍历场景中的 `FRenderable` 对象。
    - 应用全局光照参数 (Light Position, Camera Position)。
    - 提交绘制指令。

### 2.3 基于图像的照明 (IBL)
- **HDR 加载**：支持加载 `.hdr` 格式的高动态范围全景图。
- **环境光照**：
    - Diffuse: 使用 Irradiance Map (球谐或卷积)。
    - Specular: 使用 Pre-filtered Environment Map (Split-Sum 近似)。
    - BRDF LUT: 预计算的查找纹理。

### 2.4 场景管理 (Scene Management)
- **FSceneNode**：支持层级化的 TRS (Translation, Rotation, Scale) 变换。
- **自动矩阵更新**：父节点的变换会自动传播给子节点。

## 3. 使用示例
```cpp
// 1. 创建材质
auto mat = std::make_shared<FStandardPBRMaterial>();
mat->LoadShaders("shaders/StandardPBR.vert", "shaders/StandardPBR.frag");
mat->SetParameter("albedo", glm::vec3(1.0f, 0.0f, 0.0f));

// 2. 创建渲染对象
FRenderable obj;
obj.Material = mat;
obj.VertexBuffer = myVB;
obj.WorldMatrix = myNode.GetWorldMatrix();

// 3. 提交给 RenderGraph
// (目前在 main.cpp 中手动组装 scene vector)
```
