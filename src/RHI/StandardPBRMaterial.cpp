#include "StandardPBRMaterial.h"
#include "../Core/ShaderCompiler.h"
#include "Kernel/Core/Log.h"
#include "OpenGL/OpenGLResources.h"
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

namespace AEngine {

    FStandardPBRMaterial::FStandardPBRMaterial(const std::string& name)
        : m_name(name) {}

    FStandardPBRMaterial::~FStandardPBRMaterial() {
        if (m_program) glDeleteProgram(m_program);
    }

    void FStandardPBRMaterial::LoadShaders(const std::string& vertPath, const std::string& fragPath) {
        auto readFile = [](const std::string& path) {
            std::ifstream file(path);
            std::stringstream ss;
            ss << file.rdbuf();
            return ss.str();
        };

        std::string vertSrc = readFile(vertPath);
        std::string fragSrc = readFile(fragPath);

        // 1. Compile GLSL to SPIR-V via glslang
        auto& compiler = FShaderCompiler::Get();
        auto vertSpv = compiler.CompileGLSL(EShaderStage::Vertex, vertSrc);
        auto fragSpv = compiler.CompileGLSL(EShaderStage::Fragment, fragSrc);

        if (!vertSpv || !fragSpv) {
            AE_CORE_ERROR("Failed to compile PBR shaders to SPIR-V!");
            return;
        }

        // 2. Load SPIR-V into OpenGL
        // Note: Functions were manually loaded in WindowSubsystem if glad missed them
        if (!glShaderBinary || !glSpecializeShader) {
            AE_CORE_CRITICAL("OpenGL SPIR-V functions still not available!");
            return;
        }

        GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderBinary(1, &vertShader, GL_SHADER_BINARY_FORMAT_SPIR_V, vertSpv->data(), (GLsizei)(vertSpv->size() * sizeof(uint32_t)));
        glSpecializeShader(vertShader, "main", 0, nullptr, nullptr);

        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderBinary(1, &fragShader, GL_SHADER_BINARY_FORMAT_SPIR_V, fragSpv->data(), (GLsizei)(fragSpv->size() * sizeof(uint32_t)));
        glSpecializeShader(fragShader, "main", 0, nullptr, nullptr);

        // 3. Link Program
        m_program = glCreateProgram();
        glAttachShader(m_program, vertShader);
        glAttachShader(m_program, fragShader);
        glLinkProgram(m_program);

        GLint success;
        glGetProgramiv(m_program, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
            AE_CORE_ERROR("Program Linking Failed (SPIR-V): {0}", infoLog);
        } else {
            AE_CORE_INFO("PBR Program Linked Successfully using SPIR-V. ID: {0}", m_program);
        }

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
    }

    void FStandardPBRMaterial::Bind() {
        if (m_program) {
            glUseProgram(m_program);
            BindUniforms();
        }
    }

    void FStandardPBRMaterial::BindUniforms() {
            // Locations must match StandardPBR.vert/.frag layout(location=...) 不是
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(m_model));      // model
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_view));       // view
            glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(m_projection)); // projection
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(m_lightSpaceMatrix)); // lightSpaceMatrix

            glUniform3fv(20, 1, &m_albedo[0]); // albedo
            glUniform1f(21, m_metallic);       // metallic
            glUniform1f(22, m_roughness);      // roughness
            glUniform1f(23, m_ao);             // ao

            glUniform3fv(24, 1, glm::value_ptr(m_lightPosition)); // lightPosition
            glUniform3fv(25, 1, glm::value_ptr(m_lightColor));    // lightColor
            glUniform3fv(26, 1, glm::value_ptr(m_camPos));        // camPos

            // IBL Samplers
            glUniform1i(27, 0); // irradianceMap
            glUniform1i(28, 1); // prefilterMap
            glUniform1i(29, 2); // brdfLUT

            // Material Textures
            if (m_albedoMap) {
                auto* glTex = static_cast<FOpenGLTexture*>(m_albedoMap.get());
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, glTex->GetHandle());
                glUniform1i(30, 3);
                glUniform1i(32, 1); // useAlbedoMap = true
            } else {
                glUniform1i(32, 0);
            }

            if (m_normalMap) {
                auto* glTex = static_cast<FOpenGLTexture*>(m_normalMap.get());
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, glTex->GetHandle());
                glUniform1i(31, 4);
                glUniform1i(33, 1); // useNormalMap = true
            } else {
                glUniform1i(33, 0);
            }

            if (m_shadowMap) {
                auto* glTex = static_cast<FOpenGLTexture*>(m_shadowMap.get());
                glActiveTexture(GL_TEXTURE5);
                glBindTexture(GL_TEXTURE_2D, glTex->GetHandle());
                glUniform1i(34, 5);
            }
    }

    void FStandardPBRMaterial::SetParameter(const std::string& name, const FMaterialParamValue& value) {
        if (name == "albedo") m_albedo = std::get<glm::vec3>(value);
        else if (name == "metallic") m_metallic = std::get<float>(value);
        else if (name == "roughness") m_roughness = std::get<float>(value);
        else if (name == "ao") m_ao = std::get<float>(value);
        else if (name == "lightPosition") m_lightPosition = std::get<glm::vec3>(value);
        else if (name == "lightColor") m_lightColor = std::get<glm::vec3>(value);
        else if (name == "camPos") m_camPos = std::get<glm::vec3>(value);
        else if (name == "model_matrix") m_model = std::get<glm::mat4>(value);
        else if (name == "view_matrix") m_view = std::get<glm::mat4>(value);
        else if (name == "projection_matrix") m_projection = std::get<glm::mat4>(value);
        else if (name == "lightSpaceMatrix") m_lightSpaceMatrix = std::get<glm::mat4>(value);
        else if (name == "albedoMap") m_albedoMap = std::get<std::shared_ptr<IRHITexture>>(value);
        else if (name == "normalMap") m_normalMap = std::get<std::shared_ptr<IRHITexture>>(value);
        else if (name == "shadowMap") m_shadowMap = std::get<std::shared_ptr<IRHITexture>>(value);
    }

}