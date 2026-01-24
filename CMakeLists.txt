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
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,bugprone-*,performance-*,modernize-*,readability-*,-modernize-use-trailing-return-type")
else()
    message(WARNING "clang-tidy not found!")
endif()

add_executable(AEngine 
    main.cpp
    src/Core/Log.cpp
    src/Core/Engine.cpp
    src/Core/PluginManager.cpp
    src/Core/WindowSubsystem.cpp
    src/Core/FApplication.cpp
    src/Core/SceneNode.cpp
    src/Core/GeometryUtils.cpp
    src/Core/ShaderCompiler.cpp
    src/RHI/OpenGL/OpenGLResources.cpp
    src/RHI/OpenGL/OpenGLCommandBuffer.cpp
    src/RHI/OpenGL/OpenGLDevice.cpp
    src/RHI/OpenGL/OpenGLFramebuffer.cpp
    src/RHI/StandardPBRMaterial.cpp
    src/RHI/ForwardLitPass.cpp
    src/RHI/ShadowPass.cpp
    src/RHI/DeferredGeometryPass.cpp
    src/RHI/DeferredLightingPass.cpp
    src/Core/AssetLoader.cpp
    src/RHI/IBLPreprocessor.cpp
)


target_link_libraries(AEngine PRIVATE 
    spdlog::spdlog
    glfw
    glm::glm
    RTTR::Core
    assimp::assimp
    glad::glad
    imgui::imgui
    tl::expected
    OpenGL::GL
    glslang::glslang
    glslang::SPIRV
    spirv-cross-core
    spirv-cross-glsl
)
target_include_directories(AEngine PRIVATE src)

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
    src/Core/Log.cpp
    src/Core/Engine.cpp
    src/Core/PluginManager.cpp
    src/Core/WindowSubsystem.cpp
    src/Core/SceneNode.cpp
    src/Core/ShaderCompiler.cpp
)
target_link_libraries(AEngineTests PRIVATE 
    Catch2::Catch2WithMain
    spdlog::spdlog
    tl::expected
    glfw
    glad::glad
    imgui::imgui
    OpenGL::GL
    glm::glm
    glslang::glslang
    glslang::SPIRV
)
target_include_directories(AEngineTests PRIVATE src)
add_test(NAME LogTests COMMAND AEngineTests)
add_test(NAME EngineTests COMMAND AEngineTests)
add_test(NAME PluginTests COMMAND AEngineTests)
add_test(NAME SceneTests COMMAND AEngineTests)

# Plugins
add_library(StatsPanel SHARED plugins/StatsPanel/StatsPanel.cpp)
target_link_libraries(StatsPanel PRIVATE imgui::imgui)
target_include_directories(StatsPanel PRIVATE src)
