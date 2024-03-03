//
// Created by Piotr Kubala on 03/03/2024.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeParameterRandomizer.h"

#include "core/lattice/ShapeParameterRandomizingTransformer.h"
#include "core/lattice/UnitCellFactory.h"
#include "core/shapes/SphereTraits.h"


TEST_CASE("ShapeParameterRandomizingTransformer") {
    using trompeloeil::_;
    ShapeData data(SphereTraits::HardData{1});
    Lattice lattice(UnitCellFactory::createBccCell(7, data), {4, 4, 4});
    SphereTraits traits;

    SECTION("correct randomizer") {
        auto mockRandomizer = std::make_shared<MockShapeParameterRandomizer>();
        // "Randomizer" setting radius to 2
        ALLOW_CALL(*mockRandomizer, randomize("1", _)).RETURN("2");

        SECTION("correct parameter") {
            ShapeParameterRandomizingTransformer transformer("r", mockRandomizer, 1234ul);

            transformer.transform(lattice, traits);

            auto molecules = lattice.generateMolecules();
            CHECK(std::all_of(molecules.begin(), molecules.end(), [](const Shape &shape) {
                return shape.getData().as<SphereTraits::HardData>().radius == 2;
            }));
        }

        SECTION("incorrect parameter") {
            ShapeParameterRandomizingTransformer transformer("incorrect", mockRandomizer, 1234ul);

            CHECK_THROWS_MATCHES(transformer.transform(lattice, traits),
                                 TransformerException,
                                 Catch::Message("Shape parameter randomization: unknown parameter incorrect"));
        }
    }

    SECTION("randomizer failing data validation") {
        auto mockRandomizer = std::make_shared<MockShapeParameterRandomizer>();
        // "Randomizer" setting radius to 0
        ALLOW_CALL(*mockRandomizer, randomize("1", _)).RETURN("0");
        ShapeParameterRandomizingTransformer transformer("r", mockRandomizer, 1234ul);

        CHECK_THROWS_MATCHES(transformer.transform(lattice, traits),
                             TransformerException,
                             Catch::Message("Shape parameter randomization yielded malformed shape data: "
                                            "sphere's radius must be > 0"));
    }
}