# Track 规格说明 - Assimp 模型加载 (Assimp Model Loading)

## 1. 概述
本 Track 旨在集成 Assimp 库到 AEngine 的资源加载系统中，使引擎能够导入外部 3D 模型（如 OBJ, FBX, GLTF），并将其自动转换为 RHI 缓冲区和 PBR 材质资源。

## 2. 功能需求
### 2.1 模型解析与加载
- 集成 **Assimp** 库到 `FAssetLoader`。
- 支持加载多种主流 3D 格式。
- 从 `aiScene` 中递归提取所有网格体 (Mesh) 数据。
- **顶点属性支持**：
    - 位置 (vec3)
    - 法线 (vec3)
    - 纹理坐标 (vec2)
    - 切线与副切线 (vec3) - 用于法线贴图
    - 顶点颜色 (vec4)

### 2.2 自动材质与纹理处理
- 解析 Assimp 材质 (`aiMaterial`)：
    - 提取 BaseColor (Albedo) 贴图路径并加载。
    - 提取 Normal Map 贴图路径并加载。
    - 提取 Metallic/Roughness 贴图（如果存在）。
- 自动创建 `FStandardPBRMaterial` 实例并将加载的纹理应用到对应参数。

### 2.3 转换与存储
- 将解析出的几何数据直接创建为 `IRHIBuffer`（Vertex Buffer, Index Buffer）。
- 封装为 `FRenderable` 结构，以便直接提交给 Render Graph。

## 3. 技术约束
- 必须兼容现有的 `FVertex` 结构（可能需要扩展以包含切线等新属性）。
- 纹理加载需继续利用 `stb_image`。
- 坐标系处理：Assimp 默认坐标系需转换为 AEngine 坐标系。

## 4. 验收标准
- 能够通过一行代码加载外部模型文件（例如：`loader.LoadModel("path/to/model.fbx")`）。
- 加载出的模型能在窗口中正确渲染，且带有材质贴图。
- 控制台能够记录模型加载的详细信息（网格数、顶点数、材质信息等）。

## 5. 超出范围
- 骨骼动画 (Skeletal Animation) 支持。
- 复杂的材质转换逻辑（仅支持基础 PBR 属性）。
- 模型的异步并行加载。
