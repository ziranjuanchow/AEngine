#include "AssetLoader.h"
#include "Kernel/Core/Log.h"
#include "Engine.Scene/GeometryUtils.h"
#include "Engine.RHI/RHIDefinitions.h"
#include "Engine.Renderer/StandardPBRMaterial.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace AEngine {

    static std::shared_ptr<IRHITexture> LoadTextureForMaterial(IRHIDevice& device, const aiMaterial* mat, aiTextureType type, const std::string& modelPath) {
        if (mat->GetTextureCount(type) > 0) {
            aiString str;
            mat->GetTexture(type, 0, &str);
            std::filesystem::path texturePath = std::filesystem::path(modelPath).parent_path() / str.C_Str();
            
            // Fix path separator for Windows if necessary
            std::string pathStr = texturePath.string();
            // TODO: Ensure path exists or fallback
            
            return FAssetLoader::LoadTexture(device, pathStr);
        }
        return nullptr;
    }

    static FMesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const std::vector<std::shared_ptr<IMaterial>>& materials) {
        FMesh outMesh;
        outMesh.Name = mesh->mName.C_Str();

        // 1. Vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            FVertex vertex;
            
            // Position
            vertex.Position.x = mesh->mVertices[i].x;
            vertex.Position.y = mesh->mVertices[i].y;
            vertex.Position.z = mesh->mVertices[i].z;

            // Normal
            if (mesh->HasNormals()) {
                vertex.Normal.x = mesh->mNormals[i].x;
                vertex.Normal.y = mesh->mNormals[i].y;
                vertex.Normal.z = mesh->mNormals[i].z;
            } else {
                vertex.Normal = glm::vec3(0.0f);
            }

            // TexCoords
            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
                vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
            } else {
                vertex.TexCoords = glm::vec2(0.0f);
            }

            // Tangent & Bitangent
            if (mesh->HasTangentsAndBitangents()) {
                vertex.Tangent.x = mesh->mTangents[i].x;
                vertex.Tangent.y = mesh->mTangents[i].y;
                vertex.Tangent.z = mesh->mTangents[i].z;

                vertex.Bitangent.x = mesh->mBitangents[i].x;
                vertex.Bitangent.y = mesh->mBitangents[i].y;
                vertex.Bitangent.z = mesh->mBitangents[i].z;
            } else {
                vertex.Tangent = glm::vec3(0.0f);
                vertex.Bitangent = glm::vec3(0.0f);
            }

            // Color
            if (mesh->HasVertexColors(0)) {
                vertex.Color.r = mesh->mColors[0][i].r;
                vertex.Color.g = mesh->mColors[0][i].g;
                vertex.Color.b = mesh->mColors[0][i].b;
                vertex.Color.a = mesh->mColors[0][i].a;
            } else {
                vertex.Color = glm::vec4(1.0f);
            }

            outMesh.Vertices.push_back(vertex);
        }

        // 2. Indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                outMesh.Indices.push_back(face.mIndices[j]);
        }

        // 3. Material
        if (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < materials.size()) {
            outMesh.Material = materials[mesh->mMaterialIndex];
        }

        return outMesh;
    }

    static void ProcessNode(aiNode* node, const aiScene* scene, FModel& model, const std::vector<std::shared_ptr<IMaterial>>& materials) {
        // Process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model.Meshes.push_back(ProcessMesh(mesh, scene, materials));
        }
        // Then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene, model, materials);
        }
    }

    std::shared_ptr<FModel> FAssetLoader::LoadModel(IRHIDevice& device, const std::string& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, 
            aiProcess_Triangulate | 
            aiProcess_GenSmoothNormals | 
            aiProcess_FlipUVs | 
            aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            AE_CORE_ERROR("Assimp Error: {0}", importer.GetErrorString());
            return nullptr;
        }

        // Process Materials
        std::vector<std::shared_ptr<IMaterial>> materials;
        for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
            aiMaterial* aiMat = scene->mMaterials[i];
            auto mat = std::make_shared<FStandardPBRMaterial>("AssimpMat_" + std::to_string(i));
            mat->LoadShaders("shaders/StandardPBR.vert", "shaders/StandardPBR.frag");

            if (auto tex = LoadTextureForMaterial(device, aiMat, aiTextureType_DIFFUSE, path)) {
                mat->SetParameter("albedoMap", tex);
            }
            if (auto tex = LoadTextureForMaterial(device, aiMat, aiTextureType_NORMALS, path)) {
                mat->SetParameter("normalMap", tex);
            } else if (auto tex = LoadTextureForMaterial(device, aiMat, aiTextureType_HEIGHT, path)) {
                mat->SetParameter("normalMap", tex);
            }

            materials.push_back(mat);
        }

        auto model = std::make_shared<FModel>();
        ProcessNode(scene->mRootNode, scene, *model, materials);

        // Upload to GPU and create Renderables
        for (auto& mesh : model->Meshes) {
            FRenderable renderable;
            renderable.VertexBuffer = device.CreateBuffer(ERHIBufferType::Vertex, (uint32_t)(mesh.Vertices.size() * sizeof(FVertex)), ERHIBufferUsage::Static, mesh.Vertices.data());
            renderable.IndexBuffer = device.CreateBuffer(ERHIBufferType::Index, (uint32_t)(mesh.Indices.size() * sizeof(uint32_t)), ERHIBufferUsage::Static, mesh.Indices.data());
            renderable.IndexCount = (uint32_t)mesh.Indices.size();
            renderable.Material = mesh.Material;
            renderable.WorldMatrix = glm::mat4(1.0f); // Default position

            model->Renderables.push_back(renderable);
        }

        AE_CORE_INFO("Loaded model and uploaded to GPU: {0} ({1} meshes)", path, model->Meshes.size());
        return model;
    }

    std::shared_ptr<IRHITexture> FAssetLoader::LoadTexture(IRHIDevice& device, const std::string& path, bool srgb) {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4); // Force 4 channels for simplicity
        
        if (!data) {
            AE_CORE_ERROR("Failed to load texture: {0}", path);
            return nullptr;
        }

        // Determine format
        // Since we forced 4 channels, it's RGBA. 
        // Note: Our ERHIPixelFormat is limited. We might need RGBA8_SRGB in the future.
        // For now, we use RGBA8_UNORM and handle gamma in shader or assume linear flow.
        
        auto texture = device.CreateTexture(width, height, ERHIPixelFormat::RGBA8_UNORM, data);
        
        stbi_image_free(data);
        return texture;
    }

    std::shared_ptr<IRHITexture> FAssetLoader::LoadHDRTexture(IRHIDevice& device, const std::string& path) {
        stbi_set_flip_vertically_on_load(true);
        int width, height, nrComponents;
        float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
        
        if (!data) {
            AE_CORE_ERROR("Failed to load HDR image: {0}", path);
            return nullptr;
        }

        AE_CORE_INFO("Loaded HDR image: {0} ({1}x{2})", path, width, height);

        // Currently we directly use OpenGL backend implementation.
        // In a more abstract RHI, we would use IRHIDevice::CreateTexture
        auto texture = device.CreateTexture(width, height, ERHIPixelFormat::RGBA16_FLOAT, data);

        stbi_image_free(data);
        return texture;
    }

}