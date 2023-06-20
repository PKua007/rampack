//
// Created by Piotr Kubala on 20/06/2023.
//

#include <catch2/catch.hpp>

#include "mocks/MockShapeTraits.h"

#include "matchers/VectorApproxMatcher.h"

#include "core/lattice/AxisRotationRandomizingTransformer.h"
#include "core/lattice/UnitCellFactory.h"


TEST_CASE("AxisRotationRandomizingTransformer") {
    using trompeloeil::_;

    Lattice lattice(UnitCellFactory::createScCell(1), {2, 2, 2});

    MockShapeTraits traits;
    ALLOW_CALL(traits, getPrimaryAxis(_)).RETURN(Vector<3>{1, 0, 0});

    SECTION("extrinsic axis") {
        AxisRotationRandomizingTransformer transformer(Vector<3>{1, 0, 0}, 1234ul);

        transformer.transform(lattice, traits);

        for (const auto &shape : lattice.generateMolecules()) {
            auto rotatedAxis = shape.getOrientation() * Vector<3>{1, 0, 0};
            auto rotatedNonAxis = shape.getOrientation() * Vector<3>{0, 1, 0};
            CHECK_THAT(rotatedAxis, IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
            CHECK_THAT(rotatedNonAxis, !IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
        }
    }

    SECTION("intrinsic axis") {
        AxisRotationRandomizingTransformer transformer(ShapeGeometry::Axis::PRIMARY, 1234ul);

        transformer.transform(lattice, traits);

        for (const auto &shape : lattice.generateMolecules()) {
            auto rotatedAxis = shape.getOrientation() * Vector<3>{1, 0, 0};
            auto rotatedNonAxis = shape.getOrientation() * Vector<3>{0, 1, 0};
            CHECK_THAT(rotatedAxis, IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
            CHECK_THAT(rotatedNonAxis, !IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
        }
    }
}