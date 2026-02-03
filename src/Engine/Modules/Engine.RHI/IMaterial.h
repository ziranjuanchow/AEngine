#pragma once

#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <glm/glm.hpp>
#include "RHIResources.h"

namespace AEngine {

    // Material Parameter Types
    using FMaterialParamValue = std::variant<float, glm::vec3, glm::vec4, glm::mat4, std::shared_ptr<IRHITexture>>;

    struct FMaterialParameter {
        std::string Name;
        FMaterialParamValue Value;
    };

    /**
     * @brief Interface for materials.
     * 
     * Represents a shader program and its associated parameters/uniforms.
     */
    class IMaterial {
    public:
        virtual ~IMaterial() = default;

        /**
         * @brief Binds the material (shader program and uniforms) to the pipeline.
         */
        virtual void Bind() = 0;

        /**
         * @brief Sets a material parameter value.
         * @param name The name of the uniform/parameter.
         * @param value The value variant (float, vec3, mat4, texture).
         */
        virtual void SetParameter(const std::string& name, const FMaterialParamValue& value) = 0;

        /**
         * @brief Gets the name of the material.
         */
        virtual const std::string& GetName() const = 0;
    };

}
