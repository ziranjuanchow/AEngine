#include "AssetLoader.h"
#include "Log.h"
#include "../RHI/OpenGL/OpenGLResources.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace AEngine {

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
