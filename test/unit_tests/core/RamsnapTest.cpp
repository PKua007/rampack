//
// Created by pkua on 22.11.22.
//

#include <catch2/catch.hpp>

#include "core/io/RamsnapReader.h"
#include "core/io/RamsnapWriter.h"
#include "core/PeriodicBoundaryConditions.h"
#include "core/shapes/SphereTraits.h"


TEST_CASE("RamsnapReader and RamsnapWriter") {
    SphereTraits traits(0.5);
    auto pbc = std::make_unique<PeriodicBoundaryConditions>();
    std::vector<Shape> shapes;
    shapes.emplace_back(Vector<3>{0.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{4.5, 0.5, 0.5});
    shapes.emplace_back(Vector<3>{2.5, 2.5, 4.0});
    Packing packing({5, 5, 5}, std::move(shapes), std::move(pbc), traits.getInteraction());

    std::stringstream inOut;
    auto pbc2 = std::make_unique<PeriodicBoundaryConditions>();
    Packing packingRestored(std::move(pbc2));

    std::map<std::string, std::string> expectedAuxInfo = {{"key1", "value 1"}, {"key2", "value 2"}};
    RamsnapWriter writer;
    RamsnapReader reader;
    writer.write(inOut, packing, traits, expectedAuxInfo);
    auto auxInfo = reader.read(inOut, packingRestored, traits);

    REQUIRE(packing.size() == packingRestored.size());
    CHECK(packing[0] == packingRestored[0]);
    CHECK(packing[1] == packingRestored[1]);
    CHECK(packing[2] == packingRestored[2]);
    REQUIRE(auxInfo.size() == 2);
    CHECK(auxInfo["key1"] == "value 1");
    CHECK(auxInfo["key2"] == "value 2");
}