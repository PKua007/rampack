//
// Created by Piotr Kubala on 20/06/2023.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"

#include "matchers/VectorApproxMatcher.h"

#include "core/lattice/RotationRandomizingTransformer.h"
#include "core/lattice/UnitCellFactory.h"


TEST_CASE("RotationRandomizingTransformer") {
    using trompeloeil::_;

    Lattice lattice(UnitCellFactory::createScCell(1), {2, 2, 2});

    MockShapeTraits traits;
    ALLOW_CALL(traits, getPrimaryAxis(_)).RETURN(Vector<3>{1, 0, 0});

    SECTION("extrinsic axis") {
        RotationRandomizingTransformer transformer(Vector<3>{1, 0, 0}, 1234ul);

        transformer.transform(lattice, traits);

        for (const auto &shape : lattice.generateMolecules()) {
            auto rotatedAxis = shape.getOrientation() * Vector<3>{1, 0, 0};
            auto rotatedNonAxis = shape.getOrientation() * Vector<3>{0, 1, 0};
            CHECK_THAT(rotatedAxis, IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
            CHECK_THAT(rotatedNonAxis, !IsApproxEqual(Vector<3>{0, 1, 0}, 1e-12));
        }
    }

    SECTION("intrinsic axis") {
        RotationRandomizingTransformer transformer(ShapeGeometry::Axis::PRIMARY, 1234ul);

        transformer.transform(lattice, traits);

        for (const auto &shape : lattice.generateMolecules()) {
            auto rotatedAxis = shape.getOrientation() * Vector<3>{1, 0, 0};
            auto rotatedNonAxis = shape.getOrientation() * Vector<3>{0, 1, 0};
            CHECK_THAT(rotatedAxis, IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
            CHECK_THAT(rotatedNonAxis, !IsApproxEqual(Vector<3>{0, 1, 0}, 1e-12));
        }
    }

    SECTION("random axis") {
        RotationRandomizingTransformer transformer(RotationRandomizingTransformer::RANDOM_AXIS, 1234ul);

        transformer.transform(lattice, traits);

        for (const auto &shape : lattice.generateMolecules()) {
            auto rotatedVector1 = shape.getOrientation() * Vector<3>{1, 0, 0};
            auto rotatedVector2 = shape.getOrientation() * Vector<3>{0, 1, 0};
            CHECK_THAT(rotatedVector1, !IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
            CHECK_THAT(rotatedVector2, !IsApproxEqual(Vector<3>{0, 1, 0}, 1e-12));
        }
    }
}