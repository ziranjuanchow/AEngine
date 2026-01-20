# 功能说明：资源加载 (Asset Loading)

## 1. 简介
AEngine 提供了强大的资源加载系统，支持多种 3D 模型格式和高动态范围 (HDR) 图像。该系统利用 Assimp 和 stb_image 库实现，并自动将资产转换为引擎内部的 RHI 资源。

## 2. 核心功能

### 2.1 模型加载 (Assimp Integration)
- **支持格式**：OBJ, FBX, GLTF, STL 等。
- **顶点数据**：自动提取位置、法线、纹理坐标、切线、副切线和顶点颜色。
- **层级处理**：递归解析模型节点树，支持多网格 (Multi-mesh) 模型。
- **自动 RHI 转换**：加载时自动创建 `IRHIBuffer` (VBO/IBO) 并生成 `FRenderable` 列表。

### 2.2 自动材质映射
- **材质转换**：解析模型自带的材质信息 (`aiMaterial`)。
- **纹理关联**：自动查找并加载 Albedo Map 和 Normal Map 贴图。
- **实例创建**：为每个网格自动创建 `FStandardPBRMaterial` 实例。

### 2.3 图像加载 (stb_image)
- **LDR 纹理**：支持 PNG, JPG 等常用格式。
- **HDR 纹理**：支持 `.hdr` 格式全景图，用于 IBL 环境光照。

## 3. 开发者指南
### 加载模型示例：
```cpp
auto& device = ...; // 获取 RHI 设备
auto model = FAssetLoader::LoadModel(device, "assets/models/character.fbx");

if (model) {
    // 渲染模型的所有网格
    for (const auto& renderable : model->Renderables) {
        renderGraph.Submit(renderable);
    }
}
```

## 4. 注意事项
- **坐标系**：Assimp 加载时会自动处理 Y-Up 和 Z-Up 的转换。
- **纹理路径**：如果模型中的纹理路径是相对路径，系统会尝试在模型文件同级目录下进行搜索。
