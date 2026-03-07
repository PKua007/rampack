//
// Created by Codex on 22/02/2026.
//

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "mocks/MockShapeGeometry.h"

#include "matchers/VectorApproxMatcher.h"

#include "core/geometry/GeneralShapeAxis.h"


TEST_CASE("GeneralShapeAxis") {
    using trompeloeil::_;
    MockShapeGeometry geometry;
    ALLOW_CALL(geometry, getPrimaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
    ALLOW_CALL(geometry, getSecondaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{0, 1, 0});

    SECTION("constructor Vector<3> axis validation") {
        CHECK_THROWS(GeneralShapeAxis(Vector<3>{0, 0, 0}));
    }

    SECTION("constructor Vector<3> axis normalization") {
        GeneralShapeAxis axis(Vector<3>{3, 4, 0});
        CHECK_THAT(axis.getForDefaultOrientation(geometry), IsApproxEqual(Vector<3>{0.6, 0.8, 0}, 1e-12));
    }

    SECTION("for default orientation") {
        SECTION("Vector<3> axis") {
            GeneralShapeAxis axis(Vector<3>{0, 0, 1});

            CHECK_THAT(axis.getForDefaultOrientation(geometry), IsApproxEqual(Vector<3>{0, 0, 1}, 1e-12));
        }

        SECTION("shape axis") {
            CHECK_THAT(GeneralShapeAxis(ShapeGeometry::Axis::PRIMARY).getForDefaultOrientation(geometry),
                       IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
            CHECK_THAT(GeneralShapeAxis(ShapeGeometry::Axis::SECONDARY).getForDefaultOrientation(geometry),
                       IsApproxEqual(Vector<3>{0, 1, 0}, 1e-12));
            CHECK_THAT(GeneralShapeAxis(ShapeGeometry::Axis::AUXILIARY).getForDefaultOrientation(geometry),
                       IsApproxEqual(Vector<3>{0, 0, 1}, 1e-12));
        }
    }

    SECTION("for specific shape") {
        SECTION("Vector<3> axis applies shape orientation") {
            GeneralShapeAxis axis(Vector<3>{0, 0, 1});
            Shape shape({}, Matrix<3, 3>::rotation(Vector<3>{0, 1, 0}, M_PI/2));

            CHECK_THAT(axis.getForShape(geometry, shape), IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
        }

        SECTION("shape axis uses provided shape") {
            GeneralShapeAxis axis(ShapeGeometry::Axis::PRIMARY);
            Shape shape({}, Matrix<3, 3>::rotation(Vector<3>{0, 0, 1}, M_PI/2));

            CHECK_THAT(axis.getForShape(geometry, shape), IsApproxEqual(Vector<3>{0, 1, 0}, 1e-12));
        }
    }

    SECTION("move sampler name suffix") {
        CHECK(GeneralShapeAxis(ShapeGeometry::Axis::PRIMARY).getMoveSamplerNameSuffix() == "primary");
        CHECK(GeneralShapeAxis(ShapeGeometry::Axis::SECONDARY).getMoveSamplerNameSuffix() == "secondary");
        CHECK(GeneralShapeAxis(ShapeGeometry::Axis::AUXILIARY).getMoveSamplerNameSuffix() == "auxiliary");
        CHECK(GeneralShapeAxis(Vector<3>{3, 4, 0}).getMoveSamplerNameSuffix()
              == "shape,0.59999999999999998,0.80000000000000004,0");
    }
}
