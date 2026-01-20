#pragma once

#include <string>
#include <memory>
#include <vector>
#include "../RHI/IRHIDevice.h"
#include "../RHI/RHIDefinitions.h"

namespace AEngine {

    struct FMesh {
        std::vector<FVertex> Vertices;
        std::vector<uint32_t> Indices;
        std::string Name;
        // Material info will be added in Phase 2
    };

    struct FModel {
        std::vector<FMesh> Meshes;
    };

    class FAssetLoader {
    public:
        static std::shared_ptr<IRHITexture> LoadHDRTexture(const std::string& path);
        static std::shared_ptr<FModel> LoadModel(const std::string& path);
    };

}
