#include "ShaderCompiler.h"
#include "Log.h"
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <fstream>
#include <filesystem>

namespace AEngine {

    class FGLSLIncluder : public glslang::TShader::Includer {
    public:
        virtual IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override {
            return includeSystem(headerName, includerName, inclusionDepth);
        }

        virtual IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override {
            std::string path = std::filesystem::path(includerName).parent_path().string() + "/" + headerName;
            if (!std::filesystem::exists(path)) {
                // Try current directory
                path = headerName;
            }

            std::ifstream file(path);
            if (!file.is_open()) {
                AE_CORE_ERROR("Failed to include shader file: {0}", headerName);
                return nullptr;
            }

            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            
            char* contentCopy = new char[content.size() + 1];
            std::copy(content.begin(), content.end(), contentCopy);
            contentCopy[content.size()] = '\0';

            return new IncludeResult(headerName, contentCopy, content.size(), contentCopy);
        }

        virtual void releaseInclude(IncludeResult* result) override {
            if (result) {
                delete[] (char*)result->userData;
                delete result;
            }
        }
    };

    FShaderCompiler& FShaderCompiler::Get() {
        static FShaderCompiler instance;
        return instance;
    }

    FShaderCompiler::~FShaderCompiler() {
        Shutdown();
    }

    void FShaderCompiler::Init() {
        if (m_initialized) return;
        
        AE_CORE_INFO("Initializing glslang...");
        if (!glslang::InitializeProcess()) {
            AE_CORE_ERROR("Failed to initialize glslang process!");
            return;
        }
        m_initialized = true;
    }

    void FShaderCompiler::Shutdown() {
        if (!m_initialized) return;
        glslang::FinalizeProcess();
        m_initialized = false;
    }

    static EShLanguage GetLanguage(EShaderStage stage) {
        switch (stage) {
            case EShaderStage::Vertex:   return EShLangVertex;
            case EShaderStage::Fragment: return EShLangFragment;
            case EShaderStage::Compute:  return EShLangCompute;
            default: return EShLangVertex;
        }
    }

    AEngine::expected<std::vector<uint32_t>, ShaderCompilerError> FShaderCompiler::CompileGLSL(EShaderStage stage, const std::string& source) {
        if (!m_initialized) Init();

        EShLanguage lang = GetLanguage(stage);
        glslang::TShader shader(lang);

        const char* shaderStrings = source.c_str();
        shader.setStrings(&shaderStrings, 1);

        // Set default resources
        const TBuiltInResource* resources = GetDefaultResources();

        EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

        FGLSLIncluder includer;

        if (!shader.parse(resources, 100, false, messages, includer)) {
            AE_CORE_ERROR("Shader Parsing Failed:\n{0}", shader.getInfoLog());
            AE_CORE_ERROR("{0}", shader.getInfoDebugLog());
            return AEngine::unexpected(ShaderCompilerError::ParsingFailed);
        }

        glslang::TProgram program;
        program.addShader(&shader);

        if (!program.link(messages)) {
            AE_CORE_ERROR("Shader Linking Failed:\n{0}", program.getInfoLog());
            AE_CORE_ERROR("{0}", program.getInfoDebugLog());
            return AEngine::unexpected(ShaderCompilerError::LinkingFailed);
        }

        std::vector<uint32_t> spirv;
        glslang::GlslangToSpv(*program.getIntermediate(lang), spirv);

        AE_CORE_INFO("Shader compiled to SPIR-V successfully.");
        return spirv;
    }

}
