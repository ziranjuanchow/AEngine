#include <catch2/catch_all.hpp>
#include "Engine.Scene/SceneNode.h"
#include "Engine.Scene/FrustumCulling.h"
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

TEST_CASE("FrustumCulling keeps objects in camera frustum", "[Scene][Culling]") {
    using namespace AEngine;

    FRenderContext ctx;
    ctx.ViewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ctx.ProjectionMatrix = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 100.0f);
    ctx.EnableFrustumCulling = true;

    FRenderable inside;
    inside.WorldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
    inside.BoundingRadius = 1.0f;

    FRenderable outside;
    outside.WorldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 0.0f, 0.0f));
    outside.BoundingRadius = 1.0f;

    const std::vector<FRenderable> input = { inside, outside };
    const auto visible = FFrustumCulling::CullByCameraFrustum(ctx, input);

    REQUIRE(visible.size() == 1);
}

TEST_CASE("FrustumCulling can be disabled from render context", "[Scene][Culling]") {
    using namespace AEngine;

    FRenderContext ctx;
    ctx.ViewMatrix = glm::mat4(1.0f);
    ctx.ProjectionMatrix = glm::mat4(1.0f);
    ctx.EnableFrustumCulling = false;

    FRenderable a;
    FRenderable b;
    const std::vector<FRenderable> input = { a, b };

    const auto visible = FFrustumCulling::CullByCameraFrustum(ctx, input);
    REQUIRE(visible.size() == input.size());
}
