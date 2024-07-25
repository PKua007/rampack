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

    template<typename T1, typename T2>
    std::ostream &operator<<(std::ostream &out, const std::pair<T1, T2> &pair) {
        return out << "{" << pair.first << ", " << pair.second << "}";
    }
}


TEST_CASE("NamedPoint") {
    Shape shape({1, 1, 1}, Matrix<3, 3>::rotation(0, 0, M_PI/2), TheData{{1, 0, 0}});

    SECTION("empty (static)") {
        NamedPoint point;

        CHECK(point.getName().empty());
        CHECK(point.getType() == NamedPoint::Type::STATIC);
        CHECK(point.evaluateFor(NamedPoint::STATIC_TAG) == Vector<3>{0, 0, 0});
        CHECK(point.evaluateFor(shape) == Vector<3>{1, 1, 1});
        CHECK(point.evaluateFor(shape.getData()) == Vector<3>{0, 0, 0});
        CHECK(point.isValidFor(shape.getData()));
    }

    SECTION("static") {
        NamedPoint point("point", Vector<3>{1, 0, 0});

        CHECK(point.getName() == "point");
        CHECK(point.getType() == NamedPoint::Type::STATIC);
        CHECK(point.evaluateFor(NamedPoint::STATIC_TAG) == Vector<3>{1, 0, 0});
        CHECK(point.evaluateFor(shape) == Vector<3>{1, 2, 1});
        CHECK(point.evaluateFor(shape.getData()) == Vector<3>{1, 0, 0});
        CHECK(point.isValidFor(shape.getData()));
    }

    SECTION("dynamic") {
        NamedPoint point("point", [](const ShapeData &data) {
            return data.as<TheData>().point;
        });

        CHECK(point.getName() == "point");
        CHECK(point.getType() == NamedPoint::Type::DYNAMIC);
        CHECK_THROWS(point.evaluateFor(NamedPoint::STATIC_TAG));
        CHECK(point.evaluateFor(shape) == Vector<3>{1, 2, 1});
        CHECK(point.evaluateFor(shape.getData()) == Vector<3>{1, 0, 0});
        CHECK(point.isValidFor(shape.getData()));
    }

    SECTION("transient") {
        enum class PointData {
            EXISTING,
            NON_EXISTING
        };

        NamedPoint::TransientEvaluator evaluator = [](const std::string &pointName, const ShapeData &data) -> Vector<3> {
            if (data.as<PointData>() == PointData::EXISTING && pointName == "existing")
                return {1, 0, 0};
            else
                throw NoSuchNamedPointForShapeException("No point named " + pointName);
        };

        SECTION("existing point") {
            shape.setData(PointData::EXISTING);
            NamedPoint point("existing", evaluator);

            CHECK(point.getName() == "existing");
            CHECK(point.getType() == NamedPoint::Type::TRANSIENT);
            CHECK_THROWS(point.evaluateFor(NamedPoint::STATIC_TAG));
            CHECK(point.evaluateFor(shape) == Vector<3>{1, 2, 1});
            CHECK(point.evaluateFor(shape.getData()) == Vector<3>{1, 0, 0});
            CHECK(point.evaluateFor(shape.getData()) == Vector<3>{1, 0, 0});
            CHECK(point.isValidFor(shape.getData()));
        }

        SECTION("non-existing point") {
            shape.setData(PointData::NON_EXISTING);
            NamedPoint point("non-existing", evaluator);

            CHECK(point.getName() == "non-existing");
            CHECK(point.getType() == NamedPoint::Type::TRANSIENT);
            CHECK_THROWS(point.evaluateFor(NamedPoint::STATIC_TAG));
            CHECK_THROWS(point.evaluateFor(shape));
            CHECK_THROWS(point.evaluateFor(shape.getData()));
            CHECK_FALSE(point.isValidFor(shape.getData()));
        }
    }
}

TEST_CASE("ShapeGeometry: named points") {
    using trompeloeil::_;

    // We have 4 points:
    // static  "a" -> {2, 0, 0}
    // static  "b" -> {3, 0, 0}
    // dynamic "c" -> TheData::point
    // transient "d" -> {4, 0, 0} but existing only for TheData::point = {1, 0, 0}

    MockShapeGeometry geometry;

    geometry.publicRegisterStaticNamedPoint("a", {2, 0, 0});
    geometry.publicRegisterStaticNamedPoint("b", {3, 0, 0});

    geometry.publicRegisterDynamicNamedPoint("c", [](const ShapeData &data){
        return data.as<TheData>().point;
    });

    NamedPoint::TransientEvaluator evaluator = [](const std::string &name, const ShapeData &data) -> Vector<3> {
        if (data.as<TheData>().point == Vector<3>{1, 0, 0} && name == "d")
            return {4, 0, 0};
        else
            throw NoSuchNamedPointForShapeException("No point named " + name);
    };
    NamedPoint::TransientLister lister = [](const ShapeData &data) -> std::set<std::string> {
        if (data.as<TheData>().point == Vector<3>{1, 0, 0})
            return {"d"};
        else
            return {};
    };
    geometry.publicRegisterTransientNamedPoint(evaluator, lister);

    ALLOW_CALL(geometry, getGeometricOrigin(_)).RETURN(_1.getOrientation() * _1.getData().template as<TheData>().point);

    const Matrix<3, 3> zRot = {0, -1, 0,
                               1,  0, 0,
                               0,  0, 1};
    ShapeData goodData(TheData{{1, 0, 0}});
    ShapeData badData(TheData{{2, 0, 0}});
    Shape goodShape({1, 1, 1}, zRot, goodData);
    Shape badShape({1, 1, 1}, zRot, badData);

    SECTION("getNamedPoint") {
        CHECK(geometry.getNamedPoint("a").getName() == "a");
        CHECK(geometry.getNamedPoint("b").getName() == "b");
        CHECK(geometry.getNamedPoint("c").getName() == "c");
        CHECK(geometry.getNamedPoint("d").getName() == "d");
        CHECK(geometry.getNamedPoint("o").getName() == "o");
    }

    SECTION("getNamedPoints") {
        auto goodPoints = geometry.getNamedPoints(goodData);
        auto badPoints = geometry.getNamedPoints(badData);

        auto evaluatorFor = [](const ShapeData &data) {
            return [data](const NamedPoint &point) {
                return std::make_pair(point.getName(), point.evaluateFor(data));
            };
        };
        using EvaluatedPoints = std::vector<std::pair<std::string, Vector<3>>>;
        EvaluatedPoints goodPointEval, badPointEval;
        std::transform(goodPoints.begin(), goodPoints.end(), std::back_inserter(goodPointEval), evaluatorFor(goodData));
        std::transform(badPoints.begin(), badPoints.end(), std::back_inserter(badPointEval), evaluatorFor(badData));
        CHECK(goodPointEval == EvaluatedPoints{
            {"a", {2, 0, 0}}, {"b", {3, 0, 0}}, {"c", {1, 0, 0}}, {"d", {4, 0, 0}}, {"o", {1, 0, 0}}
        });
        CHECK(badPointEval == EvaluatedPoints{
            {"a", {2, 0, 0}}, {"b", {3, 0, 0}}, {"c", {2, 0, 0}}, {"o", {2, 0, 0}}
        });
    }

    SECTION("evaluateNamedPoint (static)") {
        CHECK(geometry.evaluateNamedPoint("a", NamedPoint::STATIC_TAG) == Vector<3>{2, 0, 0});
        CHECK_THROWS(geometry.evaluateNamedPoint("c", NamedPoint::STATIC_TAG));
        CHECK_THROWS(geometry.evaluateNamedPoint("d", NamedPoint::STATIC_TAG));
    }

    SECTION("evaluateNamedPoint (ShapeData)") {
        CHECK(geometry.evaluateNamedPoint("a", goodData) == Vector<3>{2, 0, 0});
        CHECK(geometry.evaluateNamedPoint("a", badData) == Vector<3>{2, 0, 0});
        CHECK(geometry.evaluateNamedPoint("c", goodData) == Vector<3>{1, 0, 0});
        CHECK(geometry.evaluateNamedPoint("c", badData) == Vector<3>{2, 0, 0});
        CHECK(geometry.evaluateNamedPoint("d", goodData) == Vector<3>{4, 0, 0});
        CHECK_THROWS(geometry.evaluateNamedPoint("d", badData));
        CHECK(geometry.evaluateNamedPoint("o", goodData) == Vector<3>{1, 0, 0});
    }

    SECTION("evaluateNamedPoint (Shape)") {
        CHECK(geometry.evaluateNamedPoint("a", goodShape) == Vector<3>{1, 3, 1});
        CHECK(geometry.evaluateNamedPoint("a", badShape) == Vector<3>{1, 3, 1});
        CHECK(geometry.evaluateNamedPoint("c", goodShape) == Vector<3>{1, 2, 1});
        CHECK(geometry.evaluateNamedPoint("c", badShape) == Vector<3>{1, 3, 1});
        CHECK(geometry.evaluateNamedPoint("d", goodShape) == Vector<3>{1, 5, 1});
        CHECK_THROWS(geometry.evaluateNamedPoint("d", badShape));
        CHECK(geometry.evaluateNamedPoint("o", goodShape) == Vector<3>{1, 2, 1});
    }

    SECTION("evaluateNamedPoints (ShapeData)") {
        auto goodPoints = geometry.evaluateNamedPoints(goodData);
        auto badPoints = geometry.evaluateNamedPoints(badData);

        CHECK(goodPoints == std::map<std::string, Vector<3>>{
                {"a", {2, 0, 0}}, {"b", {3, 0, 0}}, {"c", {1, 0, 0}}, {"d", {4, 0, 0}}, {"o", {1, 0, 0}}
        });
        CHECK(badPoints ==  std::map<std::string, Vector<3>>{
                {"a", {2, 0, 0}}, {"b", {3, 0, 0}}, {"c", {2, 0, 0}}, {"o", {2, 0, 0}}
        });
    }

    SECTION("evaluateNamedPoints (Shape)") {
        auto goodPoints = geometry.evaluateNamedPoints(goodShape);
        auto badPoints = geometry.evaluateNamedPoints(badShape);

        CHECK(goodPoints == std::map<std::string, Vector<3>>{
            {"a", {1, 3, 1}}, {"b", {1, 4, 1}}, {"c", {1, 2, 1}}, {"d", {1, 5, 1}}, {"o", {1, 2, 1}}
        });
        CHECK(badPoints ==  std::map<std::string, Vector<3>>{
            {"a", {1, 3, 1}}, {"b", {1, 4, 1}}, {"c", {1, 3, 1}}, {"o", {1, 3, 1}}
        });
    }

    SECTION("hasNonTransientNamedPoint") {
        CHECK(geometry.hasNonTransientNamedPoint("a"));
        CHECK(geometry.hasNonTransientNamedPoint("o"));
        CHECK_FALSE(geometry.hasNonTransientNamedPoint("d"));
        CHECK_FALSE(geometry.hasNonTransientNamedPoint("I'm just a poor boy, nobody loves me"));
    }

    SECTION("hasNamedPoint") {
        CHECK(geometry.hasNamedPoint("a", goodData));
        CHECK(geometry.hasNamedPoint("a", badData));
        CHECK(geometry.hasNamedPoint("d", goodData));
        CHECK_FALSE(geometry.hasNamedPoint("d", badData));
        CHECK(geometry.hasNamedPoint("o", goodData));
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