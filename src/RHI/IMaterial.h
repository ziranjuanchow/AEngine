#pragma once

#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <glm/glm.hpp>
#include "RHIResources.h"

namespace AEngine {

    // Material Parameter Types
    using FMaterialParamValue = std::variant<float, glm::vec3, glm::vec4, std::shared_ptr<IRHITexture>>;

    struct FMaterialParameter {
        std::string Name;
        FMaterialParamValue Value;
    };

    class IMaterial {
    public:
        virtual ~IMaterial() = default;

        virtual void Bind() = 0;
        virtual void SetParameter(const std::string& name, const FMaterialParamValue& value) = 0;
        virtual const std::string& GetName() const = 0;
    };

}
