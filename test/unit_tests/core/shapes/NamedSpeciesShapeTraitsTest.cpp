//
// Created by Piotr Kubala on 13/03/2024.
//

#include <catch2/catch.hpp>

#include "core/shapes/NamedSpeciesShapeTraits.h"
#include "core/shapes/SphereTraits.h"


TEST_CASE("NamedSpeciesShapeTraits") {
    using SphereData = SphereTraits::HardData;
    auto sphereTraits = std::make_shared<SphereTraits>();
    NamedSpeciesShapeTraits traits(sphereTraits);
    ShapeData smallSphere(SphereData{1});
    ShapeData largeSphere(SphereData{2});
    traits.addSpecies("small", smallSphere);
    traits.addSpecies("large", largeSphere);

    SECTION("delegates") {
        CHECK(&traits.getInteraction() == &sphereTraits->getInteraction());
        CHECK(&traits.getGeometry() == &sphereTraits->getGeometry());
        CHECK(traits.getDataManager().getComparator() == sphereTraits->getDataManager().getComparator());
        CHECK(traits.getDataManager().getShapeDataSize() == sphereTraits->getDataManager().getShapeDataSize());
        CHECK_NOTHROW(traits.getPrinter("wolfram", {}));
    }

    SECTION("hasSpecies") {
        CHECK(traits.hasSpecies(smallSphere));
        CHECK(traits.hasSpecies(largeSphere));
        CHECK_FALSE(traits.hasSpecies(ShapeData(SphereData{3})));
        CHECK(traits.hasSpecies("small"));
        CHECK(traits.hasSpecies("large"));
        CHECK_FALSE(traits.hasSpecies("nonexistent"));
    }

    SECTION("shapeDataForSpecies") {
        CHECK(traits.shapeDataForSpecies("small") == smallSphere);
        CHECK(traits.shapeDataForSpecies("large") == largeSphere);
        CHECK_THROWS(traits.shapeDataForSpecies("nonexistent"));
    }

    SECTION("default species (setDefaultSpecies)") {
        CHECK_THROWS(traits.shapeDataForDefaultSpecies());
        CHECK_THROWS(traits.setDefaultSpecies("nonexistent"));

        traits.setDefaultSpecies("large");
        CHECK(traits.shapeDataForDefaultSpecies() == largeSphere);
    }

    SECTION("default species (constructor)") {
        NamedSpeciesShapeTraits defaultedTraits(sphereTraits, "large", largeSphere);

        CHECK(defaultedTraits.shapeDataForDefaultSpecies() == largeSphere);
    }

    SECTION("default deserialize") {
        traits.setDefaultSpecies("large");

        CHECK(traits.getDataManager().defaultDeserialize({}) == largeSphere);
    }

    SECTION("serialization & deserialization") {
        const auto &manager = traits.getDataManager();

        TextualShapeData textualData{{"species", "large"}};
        CHECK(manager.serialize(largeSphere) == textualData);
        CHECK(manager.deserialize(textualData) == largeSphere);
    }
}