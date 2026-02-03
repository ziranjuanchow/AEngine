cmake_minimum_required(VERSION 3.20)

project(AEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find packages
find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(rttr CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(stb REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)

# Clang-Tidy Integration
find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if(CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    # Temporarily disabled for rapid iteration
    # set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,bugprone-*,performance-*")
else()
    message(WARNING "clang-tidy not found!")
endif()

if(MSVC)
    add_compile_options(/utf-8)
    add_definitions(-DSPDLOG_COMPILED_LIB -DSPDLOG_SHARED_LIB)
endif()

# Kernel Library (Level 0)
add_library(AEngineKernel STATIC
    src/Kernel/Core/Log.cpp
    src/Kernel/Core/PluginManager.cpp
    src/Kernel/ModuleManager/ModuleManager.cpp
)
target_link_libraries(AEngineKernel PUBLIC 
    spdlog::spdlog
    nlohmann_json::nlohmann_json
)
target_include_directories(AEngineKernel PUBLIC src/Kernel src)

# Engine Runtime Modules (Level 1)
add_library(AEngineRuntime STATIC
    src/Engine/Modules/Engine.Window/WindowModule.cpp
    src/Engine/Modules/Engine.Renderer/RenderModule.cpp
    src/RHI/SceneRenderer.cpp
    src/RHI/ForwardLitPass.cpp
    src/RHI/ShadowPass.cpp
    src/RHI/DeferredGeometryPass.cpp
    src/RHI/DeferredLightingPass.cpp
    src/RHI/PostProcessPass.cpp
    src/RHI/StandardPBRMaterial.cpp
    src/RHI/IBLPreprocessor.cpp
    src/RHI/OpenGL/OpenGLResources.cpp
    src/RHI/OpenGL/OpenGLCommandBuffer.cpp
    src/RHI/OpenGL/OpenGLDevice.cpp
    src/RHI/OpenGL/OpenGLFramebuffer.cpp
    src/Core/SceneNode.cpp
    src/Core/GeometryUtils.cpp
    src/Core/ShaderCompiler.cpp
    src/Core/AssetLoader.cpp
)
target_link_libraries(AEngineRuntime PUBLIC 
    AEngineKernel
    glfw
    glad::glad
    imgui::imgui
    glm::glm
    OpenGL::GL
    assimp::assimp
    glslang::glslang
    glslang::SPIRV
    spirv-cross-core
    spirv-cross-glsl
)
target_include_directories(AEngineRuntime PUBLIC 
    src 
    src/Engine/Modules 
    src/Engine/Modules/Engine.Renderer
)

add_executable(AEngine 
    main.cpp
    src/Core/FApplication.cpp
)
target_link_libraries(AEngine PRIVATE 
    AEngineRuntime
    AEngineKernel
    RTTR::Core
)
target_include_directories(AEngine PRIVATE src src/Engine/Modules src/Engine/Modules/Engine.Renderer)

enable_testing()
add_executable(AEngineTests 
    tests/LogTests.cpp
    tests/EngineTests.cpp
    tests/PluginTests.cpp
    tests/WindowTests.cpp
    tests/SceneTests.cpp
    tests/MaterialTests.cpp
    tests/VertexTests.cpp
    tests/ShadowTests.cpp
    tests/RHITests.cpp
)
target_link_libraries(AEngineTests PRIVATE 
    AEngineRuntime
    AEngineKernel
    Catch2::Catch2WithMain
)

target_include_directories(AEngineTests PRIVATE src src/Engine/Modules src/Engine/Modules/Engine.Renderer)
add_test(NAME LogTests COMMAND AEngineTests)
add_test(NAME EngineTests COMMAND AEngineTests)
add_test(NAME PluginTests COMMAND AEngineTests)
add_test(NAME SceneTests COMMAND AEngineTests)

# Plugins
add_library(StatsPanel SHARED plugins/StatsPanel/StatsPanel.cpp)
target_link_libraries(StatsPanel PRIVATE imgui::imgui)
target_include_directories(StatsPanel PRIVATE src)
