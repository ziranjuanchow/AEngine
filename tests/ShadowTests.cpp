#include <catch2/catch_all.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

TEST_CASE("Light Space Matrix Calculation", "[Shadow]") {
    glm::vec3 lightPos(10.0f, 10.0f, 10.0f);
    glm::vec3 target(0.0f);
    
    // Orthographic projection
    float near_plane = 1.0f, far_plane = 50.0f;
    glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(lightPos, target, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    glm::vec4 testPoint(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 projected = lightSpaceMatrix * testPoint;
    
    // Should be in clip space
    REQUIRE(projected.w == 1.0f); // Ortho projection keeps w=1 (mostly)
    // Basic check: point at target should be centered
    // This is a sanity check that GLM links correctly
}
