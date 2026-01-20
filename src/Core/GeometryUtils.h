#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "../RHI/IRHIDevice.h"
#include "../RHI/RHIDefinitions.h"

namespace AEngine {

    class FGeometryUtils {
    public:
        // Generates a sphere and returns its buffers
        static void CreateSphere(IRHIDevice& device, std::shared_ptr<IRHIBuffer>& outVB, std::shared_ptr<IRHIBuffer>& outIB, uint32_t& outIndexCount);
    };

}
