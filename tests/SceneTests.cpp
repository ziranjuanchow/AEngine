#include <catch2/catch_all.hpp>
#include "../src/Core/SceneNode.h"
#include <glm/gtc/matrix_transform.hpp>

TEST_CASE("SceneNode TRS and Hierarchy", "[Scene]") {
    using namespace AEngine;

    FSceneNode root("Root");
    root.SetPosition({ 10.0f, 0.0f, 0.0f });

    auto child = std::make_unique<FSceneNode>("Child");
    child->SetPosition({ 5.0f, 0.0f, 0.0f });
    
    SECTION("Root world matrix calculation") {
        root.UpdateWorldMatrix();
        auto worldPos = glm::vec3(root.GetWorldMatrix()[3]);
        REQUIRE(worldPos.x == Catch::Approx(10.0f));
    }

    SECTION("Child world matrix calculation (Relative Pos)") {
        root.AddChild(std::move(child));
        root.UpdateWorldMatrix();
        
        // Find the child again (it was moved)
        // For testing we can keep a raw pointer or just check the math
        // But since we used unique_ptr, the previous 'child' is now invalid.
        // We will restructure test to verify the logic.
    }
}

TEST_CASE("SceneNode Hierarchy Math", "[Scene]") {
    using namespace AEngine;
    
    FSceneNode root("Root");
    root.SetPosition({ 10.0f, 0.0f, 0.0f });

    auto child = std::make_unique<FSceneNode>("Child");
    child->SetPosition({ 5.0f, 0.0f, 0.0f });
    FSceneNode* childPtr = child.get();
    
    root.AddChild(std::move(child));
    root.UpdateWorldMatrix();

    auto childWorldPos = glm::vec3(childPtr->GetWorldMatrix()[3]);
    // 10 + 5 = 15
    REQUIRE(childWorldPos.x == Catch::Approx(15.0f));
}
