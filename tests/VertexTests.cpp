#include <catch2/catch_all.hpp>
#include <glm/glm.hpp>
#include <cstddef>

// Define expected structure locally to verify against the one in GeometryUtils
struct FExpectedVertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    glm::vec4 Color;
};

TEST_CASE("Vertex Structure Memory Layout", "[RHI]") {
    // We will include the actual header later, but for now we define expectations
    REQUIRE(sizeof(glm::vec3) == 12);
    REQUIRE(sizeof(glm::vec2) == 8);
    REQUIRE(sizeof(glm::vec4) == 16);
    
    // Position(12) + Normal(12) + TexCoords(8) + Tangent(12) + Bitangent(12) + Color(16) = 72 bytes
    REQUIRE(sizeof(FExpectedVertex) == 72);
}
