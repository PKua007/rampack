//
// Created by Codex on 22/02/2026.
//

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "mocks/MockShapeGeometry.h"

#include "matchers/VectorApproxMatcher.h"

#include "core/geometry/GeneralizedShapeAxis.h"


TEST_CASE("GeneralizedShapeAxis") {
    using trompeloeil::_;
    MockShapeGeometry geometry;
    ALLOW_CALL(geometry, getPrimaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
    ALLOW_CALL(geometry, getSecondaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{0, 1, 0});

    SECTION("constructor lab-axis validation") {
        CHECK_THROWS(GeneralizedShapeAxis(Vector<3>{0, 0, 0}));
    }

    SECTION("constructor lab-axis normalization") {
        GeneralizedShapeAxis axis(Vector<3>{3, 4, 0});
        CHECK_THAT(axis.getForDefaultOrientation(geometry), IsApproxEqual(Vector<3>{0.6, 0.8, 0}, 1e-12));
    }

    SECTION("for default orientation") {
        SECTION("vector axis") {
            GeneralizedShapeAxis axis(Vector<3>{0, 0, 1});

            CHECK_THAT(axis.getForDefaultOrientation(geometry), IsApproxEqual(Vector<3>{0, 0, 1}, 1e-12));
        }

        SECTION("shape axis") {
            CHECK_THAT(GeneralizedShapeAxis(ShapeGeometry::Axis::PRIMARY).getForDefaultOrientation(geometry),
                       IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
            CHECK_THAT(GeneralizedShapeAxis(ShapeGeometry::Axis::SECONDARY).getForDefaultOrientation(geometry),
                       IsApproxEqual(Vector<3>{0, 1, 0}, 1e-12));
            CHECK_THAT(GeneralizedShapeAxis(ShapeGeometry::Axis::AUXILIARY).getForDefaultOrientation(geometry),
                       IsApproxEqual(Vector<3>{0, 0, 1}, 1e-12));
        }
    }

    SECTION("for specific shape") {
        SECTION("vector axis applies shape orientation") {
            GeneralizedShapeAxis axis(Vector<3>{0, 0, 1});
            Shape shape({}, Matrix<3, 3>::rotation(Vector<3>{0, 1, 0}, M_PI/2));

            CHECK_THAT(axis.getForShape(geometry, shape), IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
        }

        SECTION("shape axis uses provided shape") {
            GeneralizedShapeAxis axis(ShapeGeometry::Axis::PRIMARY);
            Shape shape({}, Matrix<3, 3>::rotation(Vector<3>{0, 0, 1}, M_PI/2));

            CHECK_THAT(axis.getForShape(geometry, shape), IsApproxEqual(Vector<3>{0, 1, 0}, 1e-12));
        }
    }

    SECTION("move sampler name suffix") {
        CHECK(GeneralizedShapeAxis(ShapeGeometry::Axis::PRIMARY).getMoveSamplerNameSuffix() == "primary");
        CHECK(GeneralizedShapeAxis(ShapeGeometry::Axis::SECONDARY).getMoveSamplerNameSuffix() == "secondary");
        CHECK(GeneralizedShapeAxis(ShapeGeometry::Axis::AUXILIARY).getMoveSamplerNameSuffix() == "auxiliary");
        CHECK(GeneralizedShapeAxis(Vector<3>{3, 4, 0}).getMoveSamplerNameSuffix()
              == "0.59999999999999998,0.80000000000000004,0");
    }
}
