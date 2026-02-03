#include <catch2/catch_all.hpp>
#include "Engine.Scene/SceneNode.h"
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
}

TEST_CASE("SceneNode Properties", "[Scene]") {
    using namespace AEngine;
    
    FSceneNode node("TestNode");
    
    SECTION("Visibility") {
        REQUIRE(node.IsVisible() == true);
        node.SetVisible(false);
        REQUIRE(node.IsVisible() == false);
    }

    SECTION("Renaming") {
        REQUIRE(node.GetName() == "TestNode");
        node.SetName("NewName");
        REQUIRE(node.GetName() == "NewName");
    }
}