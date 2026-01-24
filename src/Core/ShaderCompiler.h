#pragma once

#include <string>
#include <vector>
#include <tl/expected.hpp>
#include "Engine.h"

namespace AEngine {

    enum class EShaderStage {
        Vertex,
        Fragment,
        Compute
    };

    enum class ShaderCompilerError {
        InitializationFailed,
        ParsingFailed,
        LinkingFailed,
        InvalidStage
    };

    class FShaderCompiler {
    public:
        static FShaderCompiler& Get();

        void Init();
        void Shutdown();

        AEngine::expected<std::vector<uint32_t>, ShaderCompilerError> CompileGLSL(EShaderStage stage, const std::string& source);

    private:
        FShaderCompiler() = default;
        ~FShaderCompiler();

        bool m_initialized = false;
        std::unordered_map<size_t, std::vector<uint32_t>> m_cache;
    };

}
