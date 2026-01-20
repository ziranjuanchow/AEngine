#pragma once

#include <string>
#include <memory>
#include "../RHI/IRHIDevice.h"

namespace AEngine {

    class FAssetLoader {
    public:
        static std::shared_ptr<IRHITexture> LoadHDRTexture(const std::string& path);
    };

}
