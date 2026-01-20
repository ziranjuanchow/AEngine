#include "StandardPBRMaterial.h"
#include "../Core/ShaderCompiler.h"
#include "../Core/Log.h"
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
            
            glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(m_model));
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(m_view));
            glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(m_projection));

            glUniform3fv(3, 1, &m_albedo[0]);
            glUniform1f(4, m_metallic);
            glUniform1f(5, m_roughness);
            glUniform1f(6, m_ao);

            glUniform3fv(7, 1, glm::value_ptr(m_lightPosition));
            glUniform3fv(8, 1, glm::value_ptr(m_lightColor));
            glUniform3fv(9, 1, glm::value_ptr(m_camPos));

            glUniform1i(10, 0); 
            glUniform1i(11, 1); 
            glUniform1i(12, 2); 
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
    }

}