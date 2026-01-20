#include "StandardPBRMaterial.h"
#include "../Core/ShaderCompiler.h"
#include "../Core/Log.h"
#include <fstream>
#include <sstream>

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

        auto& compiler = FShaderCompiler::Get();
        
        auto vertSpv = compiler.CompileGLSL(EShaderStage::Vertex, vertSrc);
        auto fragSpv = compiler.CompileGLSL(EShaderStage::Fragment, fragSrc);

        if (!vertSpv || !fragSpv) {
            AE_CORE_ERROR("Failed to compile PBR shaders!");
            return;
        }

        // For now, we manually create OpenGL program from SPIR-V
        // In a real RHI, this would be handled by IRHIDevice::CreatePipelineState
        
        GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderBinary(1, &vertShader, GL_SHADER_BINARY_FORMAT_SPIR_V, vertSpv->data(), vertSpv->size() * sizeof(uint32_t));
        glSpecializeShader(vertShader, "main", 0, nullptr, nullptr);

        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderBinary(1, &fragShader, GL_SHADER_BINARY_FORMAT_SPIR_V, fragSpv->data(), fragSpv->size() * sizeof(uint32_t));
        glSpecializeShader(fragShader, "main", 0, nullptr, nullptr);

        m_program = glCreateProgram();
        glAttachShader(m_program, vertShader);
        glAttachShader(m_program, fragShader);
        glLinkProgram(m_program);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
    }

    void FStandardPBRMaterial::Bind() {
        if (m_program) {
            glUseProgram(m_program);
            
            glUniform3fv(glGetUniformLocation(m_program, "albedo"), 1, &m_albedo[0]);
            glUniform1f(glGetUniformLocation(m_program, "metallic"), m_metallic);
            glUniform1f(glGetUniformLocation(m_program, "roughness"), m_roughness);
            glUniform1f(glGetUniformLocation(m_program, "ao"), m_ao);
        }
    }

    void FStandardPBRMaterial::SetParameter(const std::string& name, const FMaterialParamValue& value) {
        if (name == "albedo") m_albedo = std::get<glm::vec3>(value);
        else if (name == "metallic") m_metallic = std::get<float>(value);
        else if (name == "roughness") m_roughness = std::get<float>(value);
        else if (name == "ao") m_ao = std::get<float>(value);
    }

}
