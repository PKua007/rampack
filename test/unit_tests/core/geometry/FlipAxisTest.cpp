//
// Created by Codex on 23/02/2026.
//

#include <type_traits>
#include <stdexcept>

#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

#include "mocks/MockShapeGeometry.h"

#include "matchers/VectorApproxMatcher.h"

#include "core/geometry/FlipAxis.h"


namespace {
    void checkThatFlipAxisFlipsPrimaryAxis(const Vector<3> &expectedPrimaryAxisAfterFlip, const FlipAxis& flipAxis,
                                           const ShapeGeometry &geometry, const Shape &shape = Shape{})
    {
        const auto flipAxisForShape = flipAxis.getForShape(geometry, shape);
        const auto flip = Matrix<3, 3>::rotation(flipAxisForShape, M_PI);
        Shape rotatedShape = shape;
        rotatedShape.rotate(flip);

        CHECK_THAT(geometry.getPrimaryAxis(rotatedShape), IsApproxEqual(expectedPrimaryAxisAfterFlip, 1e-12));
    }

    const Vector<3> trivialPrimaryAxis{1, 0, 0};
    const Vector<3> trivialSecondaryAxis{0, 1, 0};
    const Vector<3> trivialAuxiliaryAxis{0, 0, 1};
    const Vector<3> rotatedBy90DegAroundZAxisTrivialPrimaryAxis{0, 1, 0};
    const Vector<3> rotatedBy90DegAroundZAxisTrivialSecondaryAxis{-1, 0, 0};

    const Vector<3> nontrivialPrimaryAxis{2./3, -2./3, 1./3};
    const Vector<3> flippedNontrivialPrimaryAxis{-2./3, 2./3, -1./3};
    const Vector<3> flippedAndRotatedBy90DegAroundZAxisNontrivialPrimaryAxis{-2./3, -2./3, -1./3};

}


TEST_CASE("FlipAxis") {
    using trompeloeil::_;

    MockShapeGeometry geometryWithAllAxes;
    ALLOW_CALL(geometryWithAllAxes, getPrimaryAxis(_)).RETURN(_1.getOrientation() * trivialPrimaryAxis);
    ALLOW_CALL(geometryWithAllAxes, getSecondaryAxis(_)).RETURN(_1.getOrientation() * trivialSecondaryAxis);

    MockShapeGeometry geometryWithPrimaryOnly;
    ALLOW_CALL(geometryWithPrimaryOnly, getPrimaryAxis(_)).RETURN(_1.getOrientation() * nontrivialPrimaryAxis);
    ALLOW_CALL(geometryWithPrimaryOnly, getSecondaryAxis(_)).THROW(std::runtime_error("no secondary axis"));

    SECTION("for default orientation") {
        SECTION("shape axis") {
            CHECK_THAT(FlipAxis(ShapeGeometry::Axis::PRIMARY).getForDefaultOrientation(geometryWithAllAxes),
                       IsApproxEqual(trivialPrimaryAxis, 1e-12));
            CHECK_THAT(FlipAxis(ShapeGeometry::Axis::SECONDARY).getForDefaultOrientation(geometryWithAllAxes),
                       IsApproxEqual(trivialSecondaryAxis, 1e-12));
            CHECK_THAT(FlipAxis(ShapeGeometry::Axis::AUXILIARY).getForDefaultOrientation(geometryWithAllAxes),
                       IsApproxEqual(trivialAuxiliaryAxis, 1e-12));
        }

        SECTION("orthogonal to primary with secondary axis available") {
            const FlipAxis flipAxis(FlipAxis::orthogonalToPrimaryTag);

            CHECK_THAT(flipAxis.getForDefaultOrientation(geometryWithAllAxes), IsApproxEqual(trivialSecondaryAxis, 1e-12));
        }

        SECTION("orthogonal to primary without secondary axis available") {
            FlipAxis flipAxis(FlipAxis::orthogonalToPrimaryTag);

            checkThatFlipAxisFlipsPrimaryAxis(flippedNontrivialPrimaryAxis, flipAxis, geometryWithPrimaryOnly);
        }
    }

    SECTION("for specific shape") {
        const Matrix<3, 3> rotationAroundZAxisBy90Deg = Matrix<3, 3>::rotation(0, 0, M_PI/2);
        const Shape shape({}, rotationAroundZAxisBy90Deg);

        SECTION("shape axis uses provided shape") {
            CHECK_THAT(FlipAxis(ShapeGeometry::Axis::PRIMARY).getForShape(geometryWithAllAxes, shape),
                IsApproxEqual(rotatedBy90DegAroundZAxisTrivialPrimaryAxis, 1e-12));
        }

        SECTION("orthogonal to primary with secondary axis available") {
            CHECK_THAT(FlipAxis(FlipAxis::orthogonalToPrimaryTag).getForShape(geometryWithAllAxes, shape),
                       IsApproxEqual(rotatedBy90DegAroundZAxisTrivialSecondaryAxis, 1e-12));
        }

        SECTION("orthogonal to primary without secondary axis available") {
            FlipAxis flipAxis(FlipAxis::orthogonalToPrimaryTag);

            checkThatFlipAxisFlipsPrimaryAxis(flippedAndRotatedBy90DegAroundZAxisNontrivialPrimaryAxis, flipAxis,
                                              geometryWithPrimaryOnly, shape);
        }
    }

    SECTION("move sampler name suffix") {
        CHECK(FlipAxis(ShapeGeometry::Axis::PRIMARY).getMoveSamplerNameSuffix() == "primary");
        CHECK(FlipAxis(ShapeGeometry::Axis::SECONDARY).getMoveSamplerNameSuffix() == "secondary");
        CHECK(FlipAxis(ShapeGeometry::Axis::AUXILIARY).getMoveSamplerNameSuffix() == "auxiliary");
        CHECK(FlipAxis(FlipAxis::orthogonalToPrimaryTag).getMoveSamplerNameSuffix() == "orth_to_primary");
    }
}
