#include "AssetLoader.h"
#include "Log.h"
#include "../RHI/OpenGL/OpenGLResources.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace AEngine {

    static FMesh ProcessMesh(aiMesh* mesh, const aiScene* scene) {
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

        return outMesh;
    }

    static void ProcessNode(aiNode* node, const aiScene* scene, FModel& model) {
        // Process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model.Meshes.push_back(ProcessMesh(mesh, scene));
        }
        // Then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene, model);
        }
    }

    std::shared_ptr<FModel> FAssetLoader::LoadModel(const std::string& path) {
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

        auto model = std::make_shared<FModel>();
        ProcessNode(scene->mRootNode, scene, *model);

        AE_CORE_INFO("Loaded model: {0} ({1} meshes)", path, model->Meshes.size());
        return model;
    }

    std::shared_ptr<IRHITexture> FAssetLoader::LoadHDRTexture(const std::string& path) {
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
        auto texture = std::make_shared<FOpenGLTexture>(width, height, ERHIPixelFormat::RGBA16_FLOAT, data);

        stbi_image_free(data);
        return texture;
    }

}
