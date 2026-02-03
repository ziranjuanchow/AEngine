#pragma once

#include <vector>
#include <memory>
#include "Engine.RHI/RHIDefinitions.h"
#include "Engine.RHI/IRHIDevice.h"

namespace AEngine {

    class FGeometryUtils {
    public:
        // Generates a sphere and returns its buffers
        static void CreateSphere(IRHIDevice& device, std::shared_ptr<IRHIBuffer>& outVB, std::shared_ptr<IRHIBuffer>& outIB, uint32_t& outIndexCount);
        static void CreateQuad(IRHIDevice& device, std::shared_ptr<IRHIBuffer>& outVB, std::shared_ptr<IRHIBuffer>& outIB, uint32_t& outIndexCount);
    };

}
