cmake_minimum_required(VERSION 3.20)

project(AEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(rttr CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(tl-expected CONFIG REQUIRED)

add_executable(AEngine 
    main.cpp
    src/Core/Log.cpp
    src/Core/Engine.cpp
)

target_link_libraries(AEngine PRIVATE 
    spdlog::spdlog
    glfw
    glm::glm
    RTTR::Core
    assimp::assimp
    imgui::imgui
    tl::expected
)
target_include_directories(AEngine PRIVATE src)

enable_testing()
add_executable(AEngineTests 
    tests/LogTests.cpp
    tests/EngineTests.cpp
    src/Core/Log.cpp
    src/Core/Engine.cpp
)
target_link_libraries(AEngineTests PRIVATE 
    Catch2::Catch2WithMain
    spdlog::spdlog
    tl::expected
)
target_include_directories(AEngineTests PRIVATE src)
add_test(NAME LogTests COMMAND AEngineTests)
add_test(NAME EngineTests COMMAND AEngineTests)
