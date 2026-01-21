#pragma once

#include <string>
#include <memory>
#include <vector>
#include "../RHI/IRHIDevice.h"
#include "../RHI/RHIDefinitions.h"
#include "../RHI/IMaterial.h"
#include "../RHI/RenderGraph.h"

namespace AEngine {

    struct FMesh {
        std::vector<FVertex> Vertices;
        std::vector<uint32_t> Indices;
        std::string Name;
        std::shared_ptr<IMaterial> Material;
    };

    struct FModel {
        std::vector<FMesh> Meshes;
        std::vector<FRenderable> Renderables;
    };

    class FAssetLoader {
    public:
        static std::shared_ptr<IRHITexture> LoadHDRTexture(const std::string& path);
        static std::shared_ptr<IRHITexture> LoadTexture(const std::string& path, bool srgb = false);
        static std::shared_ptr<FModel> LoadModel(IRHIDevice& device, const std::string& path);
    };

}
