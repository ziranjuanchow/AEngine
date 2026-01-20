#pragma once

#include "IMaterial.h"
#include <glad/glad.h>

namespace AEngine {

    class FStandardPBRMaterial : public IMaterial {
    public:
        FStandardPBRMaterial(const std::string& name = "StandardPBR");
        virtual ~FStandardPBRMaterial();

        virtual void Bind() override;
        virtual void SetParameter(const std::string& name, const FMaterialParamValue& value) override;
        virtual const std::string& GetName() const override { return m_name; }

        void LoadShaders(const std::string& vertPath, const std::string& fragPath);

    private:
        std::string m_name;
        GLuint m_program = 0;

        // Default parameters
        glm::vec3 m_albedo{ 1.0f };
        float m_metallic = 0.0f;
        float m_roughness = 0.5f;
        float m_ao = 1.0f;
    };

}
