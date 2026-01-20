#include <catch2/catch_all.hpp>
#include "../src/RHI/MaterialInstance.h"

TEST_CASE("Material System Interface", "[Material]") {
    using namespace AEngine;

    SECTION("Parameter Setting") {
        FMaterialInstance mat(nullptr, "TestMat");
        mat.SetParameter("BaseColor", glm::vec3(1.0f, 0.0f, 0.0f));
        REQUIRE(mat.GetName() == "TestMat");
    }
}
