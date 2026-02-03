#pragma once

#include <memory>
#include <vector>
#include <string>
#include <glad/glad.h>
#include "RHIResources.h"

namespace AEngine {

    struct FIBLData {
        std::shared_ptr<IRHITexture> EnvCubemap;
        std::shared_ptr<IRHITexture> IrradianceMap;
        std::shared_ptr<IRHITexture> PrefilterMap;
        std::shared_ptr<IRHITexture> BrdfLUT;
    };

    class FIBLPreprocessor {
    public:
        static FIBLData Process(std::shared_ptr<IRHITexture> hdrTexture);

    private:
        // Internal helper to render to cubemap faces
        static void RenderToCubemap(GLuint program, GLuint targetCubemap, uint32_t size, int mip = 0);
    };

}
