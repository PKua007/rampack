//
// Created by pkua on 06.11.22.
//

#include <catch2/catch.hpp>
#include <ZipIterator.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/ShapeGeometry.h"


namespace {
    class HelperShapeGeometry : public ShapeGeometry {
    public:
        [[nodiscard]] double getVolume() const override { return 0; }

        [[nodiscard]] Vector<3> getGeometricOrigin(const Shape &shape) const override {
            return shape.getOrientation() * Vector<3>{1, 0, 0};
        }

        void publicRegisterNamedPoint(const std::string &name, const Vector<3> &point) {
            this->registerNamedPoint(name, point);
        }
        void publicRegisterNamedPoints(const NamedPoints &points) { this->registerNamedPoints(points); }
        void publicMoveNamedPoints(const Vector<3> &translation) { this->moveNamedPoints(translation); }
    };
}


TEST_CASE("ShapeGeometry: named points") {
    HelperShapeGeometry geometry;

    geometry.publicRegisterNamedPoint("z", {0, 1, 0});
    geometry.publicRegisterNamedPoints({{"c", {0, 2, 0}}, {"t", {0, 3, 0}}});

    SECTION("getNamedPoint(s)") {
        ShapeGeometry::NamedPoints expected{{"cm", {0, 0, 0}}, {"o",  {1, 0, 0}}, {"z",  {0, 1, 0}},
                                            {"c",  {0, 2, 0}}, {"t",  {0, 3, 0}}};
        CHECK_THAT(geometry.getNamedPoint("cm"), IsApproxEqual(expected[0].second, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("o"), IsApproxEqual(expected[1].second, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("z"), IsApproxEqual(expected[2].second, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("c"), IsApproxEqual(expected[3].second, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("t"), IsApproxEqual(expected[4].second, 1e-12));
        CHECK_THROWS(geometry.getNamedPoint("notexistent"));
        auto actual = geometry.getNamedPoints();
        REQUIRE(actual.size() == 5);
        for (const auto &[actualItem, expectedItem]: Zip(actual, expected)) {
            CHECK(actualItem.first == expectedItem.first);
            CHECK_THAT(actualItem.second, IsApproxEqual(expectedItem.second, 1e-12));
        }
    }

    SECTION("getNamedPointForShape") {
        Shape shape({0, 1, 0}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI/2));
        CHECK_THAT(geometry.getNamedPointForShape("o", shape), IsApproxEqual({0, 2, 0}, 1e-12));
    }

    SECTION("moveNamedPoint") {
        geometry.publicMoveNamedPoints({1, 0, 0});
        CHECK_THAT(geometry.getNamedPoint("cm"), IsApproxEqual({0, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("o"), IsApproxEqual({1, 0, 0}, 1e-12));
        CHECK_THAT(geometry.getNamedPoint("z"), IsApproxEqual({1, 1, 0}, 1e-12));
    }
}