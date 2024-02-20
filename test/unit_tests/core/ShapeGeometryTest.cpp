//
// Created by pkua on 06.11.22.
//

#include <catch2/catch.hpp>
#include <ZipIterator.hpp>

#include "mocks/MockShapeGeometry.h"

#include "matchers/VectorApproxMatcher.h"

#include "core/ShapeGeometry.h"


namespace {
    struct TheData {
        Vector<3> point;

        friend bool operator==(const TheData &lhs, const TheData &rhs) { return lhs.point == rhs.point; }
    };
}


TEST_CASE("ShapeGeometry::NamedPoint") {
    Shape shape({1, 1, 1}, Matrix<3, 3>::rotation(0, 0, M_PI/2), TheData{{1, 0, 0}});

    SECTION("empty") {
        ShapeGeometry::NamedPoint point;

        CHECK(point.getName().empty());
        CHECK(point.isStatic());
        CHECK_FALSE(point.isDynamic());
        CHECK(point.forStatic() == Vector<3>{0, 0, 0});
        CHECK(point.forShape(shape) == Vector<3>{1, 1, 1});
        CHECK(point.forShapeData(shape.getData()) == Vector<3>{0, 0, 0});
    }

    SECTION("static") {
        ShapeGeometry::NamedPoint point("point", Vector<3>{1, 0, 0});

        CHECK(point.getName() == "point");
        CHECK(point.isStatic());
        CHECK_FALSE(point.isDynamic());
        CHECK(point.forStatic() == Vector<3>{1, 0, 0});
        CHECK(point.forShape(shape) == Vector<3>{1, 2, 1});
        CHECK(point.forShapeData(shape.getData()) == Vector<3>{1, 0, 0});
    }

    SECTION("dynamic") {
        ShapeGeometry::NamedPoint point("point", [](const ShapeData &data) {
            return data.as<TheData>().point;
        });

        CHECK(point.getName() == "point");
        CHECK_FALSE(point.isStatic());
        CHECK(point.isDynamic());
        CHECK_THROWS(point.forStatic());
        CHECK(point.forShape(shape) == Vector<3>{1, 2, 1});
        CHECK(point.forShapeData(shape.getData()) == Vector<3>{1, 0, 0});
    }
}


TEST_CASE("ShapeGeometry: named points") {
    using trompeloeil::_;

    // We have 3 points:
    // static  "a" -> {2, 0, 0}
    // static  "b" -> {3, 0, 0}
    // dynamic "c" -> TheData::point
    // Static points are registered in 3 ways

    using NamedPoint = ShapeGeometry::NamedPoint;
    using NameRegistrarPair = std::pair<std::string, std::function<void(MockShapeGeometry&)>>;
    auto [staticRegisteringMethod, registerStaticPoints] = GENERATE(
        NameRegistrarPair("registerStaticNamedPoint", [](MockShapeGeometry &geometry) {
            geometry.publicRegisterStaticNamedPoint("a", {2, 0, 0});
            geometry.publicRegisterStaticNamedPoint("b", {3, 0, 0});
        }),
        NameRegistrarPair("registerNamedPoint", [](MockShapeGeometry &geometry) {
            geometry.publicRegisterNamedPoint(NamedPoint("a", Vector<3>{2, 0, 0}));
            geometry.publicRegisterNamedPoint(NamedPoint("b", Vector<3>{3, 0, 0}));
        }),
        NameRegistrarPair("registerNamedPoints", [](MockShapeGeometry &geometry) {
            geometry.publicRegisterNamedPoints(
                {{"a", Vector<3>{2, 0, 0}}, {"b", Vector<3>{3, 0, 0}}
            });
        })
    );

    MockShapeGeometry geometry;
    registerStaticPoints(geometry);
    geometry.publicRegisterNamedPoint(NamedPoint("c", [](const ShapeData &data){
        return data.as<TheData>().point;
    }));
    ALLOW_CALL(geometry, getGeometricOrigin(_)).RETURN(_1.getOrientation() * _1.getData().template as<TheData>().point);

    DYNAMIC_SECTION("static registration: " << staticRegisteringMethod) {
        SECTION("getNamedPoint") {
            CHECK(geometry.getNamedPoint("a").getName() == "a");
            CHECK(geometry.getNamedPoint("b").getName() == "b");
            CHECK(geometry.getNamedPoint("c").getName() == "c");
            CHECK(geometry.getNamedPoint("o").getName() == "o");
        }

        SECTION("hasNamedPoint") {
            CHECK(geometry.hasNamedPoint("a"));
            CHECK(geometry.hasNamedPoint("o"));
            CHECK_FALSE(geometry.hasNamedPoint("I'm just a poor boy, nobody loves me"));
        }

        SECTION("getNamedPointForStatic") {
            CHECK_THAT(geometry.getNamedPointForStatic("a"), IsApproxEqual(Vector<3>{2, 0, 0}, 1e-12));
            CHECK_THROWS(geometry.getNamedPointForStatic("c"));
        }

        SECTION("getNamedPointForShape") {
            Shape shape({1, 1, 1}, Matrix<3, 3>::rotation(0, 0, M_PI/2), ShapeData(TheData{{1, 0, 0}}));

            CHECK_THAT(geometry.getNamedPointForShape("a", shape), IsApproxEqual(Vector<3>{1, 3, 1}, 1e-12));
            CHECK_THAT(geometry.getNamedPointForShape("c", shape), IsApproxEqual(Vector<3>{1, 2, 1}, 1e-12));
            CHECK_THAT(geometry.getNamedPointForShape("o", shape), IsApproxEqual(Vector<3>{1, 2, 1}, 1e-12));
        }

        SECTION("getNamedPointForData") {
            ShapeData data(TheData{{1, 0, 0}});

            CHECK_THAT(geometry.getNamedPointForData("a", data), IsApproxEqual(Vector<3>{2, 0, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPointForData("c", data), IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPointForData("o", data), IsApproxEqual(Vector<3>{1, 0, 0}, 1e-12));
        }

        SECTION("moveStaticNamedPoints") {
            geometry.publicMoveStaticNamedPoints({1, 0, 0});

            CHECK_THAT(geometry.getNamedPointForStatic("a"), IsApproxEqual(Vector<3>{3, 0, 0}, 1e-12));
            CHECK_THAT(geometry.getNamedPointForStatic("b"), IsApproxEqual(Vector<3>{4, 0, 0}, 1e-12));
        }
    }
}

TEST_CASE("ShapeGeometry: flip axis") {
    using trompeloeil::_;

    MockShapeGeometry geometry;

    SECTION("primary + secondary") {
        ALLOW_CALL(geometry, getPrimaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{0, 0, 1});
        ALLOW_CALL(geometry, getSecondaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{1, 0, 0});
        Shape shape({}, Matrix<3, 3>::rotation(0, 0, M_PI/2));

        CHECK_THAT(geometry.findFlipAxis(shape), IsApproxEqual(Vector<3>{0, 1, 0}, 1e-12));
    }

    SECTION("only primary") {
        ALLOW_CALL(geometry, getPrimaryAxis(_)).RETURN(_1.getOrientation() * Vector<3>{2./3, -2./3, 1./3});
        ALLOW_CALL(geometry, getSecondaryAxis(_)).THROW(std::runtime_error("no secondary axis"));
        Shape shape({}, Matrix<3, 3>::rotation(0, 0, M_PI/2));

        // We do not care how the flip axis is chosen - the only requirement is for it to flip the sign of the primary
        // axis
        auto flipAxis = geometry.findFlipAxis(shape);
        auto rotation = Matrix<3, 3>::rotation(flipAxis, M_PI);
        shape.rotate(rotation);

        CHECK_THAT(geometry.getPrimaryAxis(shape), IsApproxEqual(Vector<3>{-2./3, -2./3, -1./3}, 1e-12));
    }
}