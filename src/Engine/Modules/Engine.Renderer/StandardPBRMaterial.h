#pragma once

#include "IMaterial.h"
#include <glad/glad.h>

namespace AEngine {

    class FStandardPBRMaterial : public IMaterial {
    public:
        FStandardPBRMaterial(const std::string& name = "StandardPBR");
        virtual ~FStandardPBRMaterial();

        virtual void Bind() override;
        // Binds parameters to the currently active program (must have compatible locations)
        void BindUniforms();
        virtual void SetParameter(const std::string& name, const FMaterialParamValue& value) override;
        virtual const std::string& GetName() const override { return m_name; }

        glm::vec3 GetAlbedo() const { return m_albedo; }
        float GetMetallic() const { return m_metallic; }
        float GetRoughness() const { return m_roughness; }
        float GetAO() const { return m_ao; }

        void LoadShaders(const std::string& vertPath, const std::string& fragPath);

    private:
        std::string m_name;
        GLuint m_program = 0;

        // Default parameters
        glm::vec3 m_albedo{ 1.0f };
        float m_metallic = 0.0f;
        float m_roughness = 0.5f;
        float m_ao = 1.0f;

        // Textures
        std::shared_ptr<IRHITexture> m_albedoMap;
        std::shared_ptr<IRHITexture> m_normalMap;
        int m_useAlbedoMap = 0;
        int m_useNormalMap = 0;

        std::shared_ptr<IRHITexture> m_shadowMap;
        glm::mat4 m_lightSpaceMatrix{ 1.0f };

        // Global context parameters
        glm::vec3 m_lightPosition{ 0.0f };
        glm::vec3 m_lightColor{ 1.0f };
        glm::vec3 m_camPos{ 0.0f };

        glm::mat4 m_model{ 1.0f };
        glm::mat4 m_view{ 1.0f };
        glm::mat4 m_projection{ 1.0f };
    };

}
