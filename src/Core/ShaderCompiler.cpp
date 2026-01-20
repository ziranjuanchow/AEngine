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

    static TBuiltInResource InitResources() {
        TBuiltInResource Resources;
        Resources.maxLights = 32;
        Resources.maxClipPlanes = 6;
        Resources.maxTextureUnits = 32;
        Resources.maxTextureCoords = 32;
        Resources.maxVertexAttribs = 64;
        Resources.maxVertexUniformComponents = 4096;
        Resources.maxVaryingFloats = 64;
        Resources.maxVertexTextureImageUnits = 32;
        Resources.maxCombinedTextureImageUnits = 80;
        Resources.maxTextureImageUnits = 32;
        Resources.maxFragmentUniformComponents = 4096;
        Resources.maxDrawBuffers = 32;
        Resources.maxVertexUniformVectors = 128;
        Resources.maxVaryingVectors = 8;
        Resources.maxFragmentUniformVectors = 16;
        Resources.maxVertexOutputVectors = 16;
        Resources.maxFragmentInputVectors = 15;
        Resources.minProgramTexelOffset = -8;
        Resources.maxProgramTexelOffset = 7;
        Resources.maxClipDistances = 8;
        Resources.maxComputeWorkGroupCountX = 65535;
        Resources.maxComputeWorkGroupCountY = 65535;
        Resources.maxComputeWorkGroupCountZ = 65535;
        Resources.maxComputeWorkGroupSizeX = 1024;
        Resources.maxComputeWorkGroupSizeY = 1024;
        Resources.maxComputeWorkGroupSizeZ = 64;
        Resources.maxComputeUniformComponents = 1024;
        Resources.maxComputeTextureImageUnits = 16;
        Resources.maxComputeImageUniforms = 8;
        Resources.maxComputeAtomicCounters = 8;
        Resources.maxComputeAtomicCounterBuffers = 1;
        Resources.maxVaryingComponents = 60;
        Resources.maxVertexOutputComponents = 64;
        Resources.maxGeometryInputComponents = 64;
        Resources.maxGeometryOutputComponents = 128;
        Resources.maxFragmentInputComponents = 128;
        Resources.maxImageUnits = 8;
        Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
        Resources.maxCombinedShaderOutputResources = 8;
        Resources.maxImageSamples = 0;
        Resources.maxVertexImageUniforms = 0;
        Resources.maxTessControlImageUniforms = 0;
        Resources.maxTessEvaluationImageUniforms = 0;
        Resources.maxGeometryImageUniforms = 0;
        Resources.maxFragmentImageUniforms = 8;
        Resources.maxCombinedImageUniforms = 8;
        Resources.maxGeometryTextureImageUnits = 16;
        Resources.maxGeometryOutputVertices = 256;
        Resources.maxGeometryTotalOutputComponents = 1024;
        Resources.maxGeometryUniformComponents = 1024;
        Resources.maxGeometryVaryingComponents = 64;
        Resources.maxTessControlInputComponents = 128;
        Resources.maxTessControlOutputComponents = 128;
        Resources.maxTessControlTextureImageUnits = 16;
        Resources.maxTessControlUniformComponents = 1024;
        Resources.maxTessControlTotalOutputComponents = 4096;
        Resources.maxTessEvaluationInputComponents = 128;
        Resources.maxTessEvaluationOutputComponents = 128;
        Resources.maxTessEvaluationTextureImageUnits = 16;
        Resources.maxTessEvaluationUniformComponents = 1024;
        Resources.maxTessPatchComponents = 120;
        Resources.maxPatchVertices = 32;
        Resources.maxTessGenLevel = 64;
        Resources.maxViewports = 16;
        Resources.maxVertexAtomicCounters = 0;
        Resources.maxTessControlAtomicCounters = 0;
        Resources.maxTessEvaluationAtomicCounters = 0;
        Resources.maxGeometryAtomicCounters = 0;
        Resources.maxFragmentAtomicCounters = 8;
        Resources.maxCombinedAtomicCounters = 8;
        Resources.maxAtomicCounterBindings = 1;
        Resources.maxVertexAtomicCounterBuffers = 0;
        Resources.maxTessControlAtomicCounterBuffers = 0;
        Resources.maxTessEvaluationAtomicCounterBuffers = 0;
        Resources.maxGeometryAtomicCounterBuffers = 0;
        Resources.maxFragmentAtomicCounterBuffers = 1;
        Resources.maxCombinedAtomicCounterBuffers = 1;
        Resources.maxAtomicCounterBufferSize = 16384;
        Resources.maxTransformFeedbackBuffers = 4;
        Resources.maxTransformFeedbackInterleavedComponents = 64;
        Resources.maxCullDistances = 8;
        Resources.maxCombinedClipAndCullDistances = 8;
        Resources.maxSamples = 4;
        Resources.maxMeshOutputVerticesNV = 256;
        Resources.maxMeshOutputPrimitivesNV = 512;
        Resources.maxMeshWorkGroupSizeX_NV = 32;
        Resources.maxMeshWorkGroupSizeY_NV = 1;
        Resources.maxMeshWorkGroupSizeZ_NV = 1;
        Resources.maxTaskWorkGroupSizeX_NV = 32;
        Resources.maxTaskWorkGroupSizeY_NV = 1;
        Resources.maxTaskWorkGroupSizeZ_NV = 1;
        Resources.maxMeshViewCountNV = 4;
        Resources.maxDualSourceDrawBuffersEXT = 1;

        Resources.limits.nonInductiveForLoops = 1;
        Resources.limits.whileLoops = 1;
        Resources.limits.doWhileLoops = 1;
        Resources.limits.generalUniformIndexing = 1;
        Resources.limits.generalAttributeMatrixVectorIndexing = 1;
        Resources.limits.generalVaryingIndexing = 1;
        Resources.limits.generalSamplerIndexing = 1;
        Resources.limits.generalVariableIndexing = 1;
        Resources.limits.generalConstantMatrixVectorIndexing = 1;

        return Resources;
    }

    static const TBuiltInResource DefaultTBuiltInResource = InitResources();

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
        const TBuiltInResource* resources = &DefaultTBuiltInResource;

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
