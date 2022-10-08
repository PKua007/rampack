//
// Created by Michal Ciesla on 9/4/22.
//

#include <catch2/catch.hpp>

#include "core/shapes/XenoCollideTraits.h"
#include "core/FreeBoundaryConditions.h"
#include "core/PeriodicBoundaryConditions.h"

#include "utils/Assertions.h"

#include "matchers/VectorApproxMatcher.h"
#include "geometry/xenocollide/XCBodyBuilder.h"
#include "geometry/xenocollide/XCPrimitives.h"


namespace {
    class XenoCollideSpherocylinderTraits : public XenoCollideTraits<XenoCollideSpherocylinderTraits> {
    private:
        static double getStaticVolume(double l, double r) {
            return M_PI * r * r * (4.0 / 3.0 * r + l);
        }

        static std::shared_ptr<AbstractXCGeometry> createShapeModel(double l, double r) {
            Expects(r > 0);
            Expects(l > 0);

            XCBodyBuilder bb;
            bb.sphere(r);
            bb.move(-l/2, 0, 0);
            bb.sphere(r);
            bb.move(l/2, 0, 0);
            bb.wrap();
            return bb.releaseCollideGeometry();
        }

        std::shared_ptr<AbstractXCGeometry> shapeModel;

    public:
        XenoCollideSpherocylinderTraits(double l, double r)
                : XenoCollideTraits({1, 0, 0}, {0, 0, 1}, {0, 0, 0}, getStaticVolume(l, r),
                                    {{"beg", {-l / 2, 0, 0}},
                                     {"end", {l / 2,  0, 0}}}),
                  shapeModel{XenoCollideSpherocylinderTraits::createShapeModel(l, r)}
        { }

        [[nodiscard]] const AbstractXCGeometry &getCollideGeometry([[maybe_unused]] std::size_t idx = 0) const {
            return *this->shapeModel;
        }
    };

    class XenoCollideDimerTraits : public XenoCollideTraits<XenoCollideDimerTraits> {
    private:
        static double getStaticVolume(double r1, double r2) {  return 4*M_PI/3 * (r1*r1*r1 + r1*r2*r2); }

        std::vector<CollideSphere> shapeModels;
        std::vector<Vector<3>> interactionCentres;

    public:
        XenoCollideDimerTraits(double r1, double r2, double x2)
                : XenoCollideTraits({1, 0, 0}, {0, 0, 1}, {0, 0, 0}, getStaticVolume(r1, r2)),
                  shapeModels{CollideSphere(r1), CollideSphere(r2)}, interactionCentres{{0, 0, 0}, {x2, 0, 0}}
        { }

        [[nodiscard]] const AbstractXCGeometry &getCollideGeometry(std::size_t idx) const {
            return shapeModels.at(idx);
        }

        [[nodiscard]] const ShapePrinter &getPrinter() const override { return *this; }

        [[nodiscard]] std::vector<Vector<3>> getInteractionCentres() const override { return this->interactionCentres; }
    };
}

TEST_CASE("XenoCollide: spherocylinder overlap") {
    FreeBoundaryConditions fbc;
    double l = 3, r = 2;
    XenoCollideSpherocylinderTraits traits(l, r);

    Shape sc1{};
    Shape sc2{};

    // Cases are found visually using Mathematica. See wolfram/spheroc_test.nb
    SECTION("sphere-sphere") {
        sc1.rotate({  0.92669949443125, -0.3009522885099318,   0.2250683608628711,
                      0.3235062902233933,   0.943614995716347, -0.07024542721869537,
                      -0.191237358292679,  0.1379074323590797,    0.971807497858173});
        sc2.rotate({0.1017126282787795, -0.2093565370521866,  0.972535028491077,
                    0.5191108031629524,   0.845122866944617,  0.127637431056981,
                    -0.848633322046771,  0.4918711011645504, 0.1946389081120092});

        SECTION("overlap") {
            sc1.translate(Vector<3>{1, 2, 3} * 0.95, fbc);
            sc2.translate(Vector<3>{-3.5, 2.9, -0.5} * 0.95, fbc);

            CHECK(traits.overlapBetweenShapes(sc1, sc2, fbc));
            CHECK(traits.overlapBetweenShapes(sc2, sc1, fbc));
        }

        SECTION("no overlap") {
            sc1.translate(Vector<3>{1, 2, 3} * 1.05, fbc);
            sc2.translate(Vector<3>{-3.5, 2.9, -0.5} * 1.05, fbc);

            CHECK_FALSE(traits.overlapBetweenShapes(sc1, sc2, fbc));
            CHECK_FALSE(traits.overlapBetweenShapes(sc2, sc1, fbc));
        }
    }

    SECTION("sphere-cylinder") {
        sc1.rotate({   0.936848795202308, -0.03157560239884583,  0.34830635403497,
                    -0.03157560239884583,    0.984212198800577, 0.174153177017485,
                       -0.34830635403497,   -0.174153177017485, 0.921060994002885});
        sc2.rotate({0.07918504528890558, 0.3698006820181268, -0.925730621823391,
                    -0.2008438095940726,  0.915521563787973,  0.348542723904781,
                      0.976417683550607, 0.1583278933673811, 0.1467677942585273});

        SECTION("overlap") {
            sc1.translate(Vector<3>{1, 2, 3} * 0.95, fbc);
            sc2.translate(Vector<3>{-0.5, 4, -2} * 0.95, fbc);

            CHECK(traits.overlapBetweenShapes(sc1, sc2, fbc));
            CHECK(traits.overlapBetweenShapes(sc2, sc1, fbc));
        }

        SECTION("no overlap") {
            sc1.translate(Vector<3>{1, 2, 3} * 1.05, fbc);
            sc2.translate(Vector<3>{-0.5, 4, -2} * 1.05, fbc);

            CHECK_FALSE(traits.overlapBetweenShapes(sc1, sc2, fbc));
            CHECK_FALSE(traits.overlapBetweenShapes(sc2, sc1, fbc));
        }
    }

    SECTION("cylinder-cylinder") {
        sc1.rotate({  0.921060994002885,   0.1446263415347036,    0.361565853836759,
                    -0.1446263415347036,    0.989111861241777, -0.02722034689555686,
                     -0.361565853836759, -0.02722034689555686,    0.931949132761108});
        sc2.rotate({ 0.4161880560663263, 0.5570946854720225, -0.7186327388914037,
                     -0.902545539870646, 0.1570999152673587, -0.4009129145869377,
                    -0.1104493116652922,  0.815453939865286,  0.5681864320017205});

        SECTION("overlap") {
            sc1.translate(Vector<3>{1, 2, 3} * 0.95, fbc);
            sc2.translate(Vector<3>{-0.5, 1.7, -0.8} * 0.95, fbc);

            CHECK(traits.overlapBetweenShapes(sc1, sc2, fbc));
            CHECK(traits.overlapBetweenShapes(sc2, sc1, fbc));
        }

        SECTION("no overlap") {
            sc1.translate(Vector<3>{1, 2, 3} * 1.05, fbc);
            sc2.translate(Vector<3>{-0.5, 1.7, -0.8} * 1.05, fbc);

            CHECK_FALSE(traits.overlapBetweenShapes(sc1, sc2, fbc));
            CHECK_FALSE(traits.overlapBetweenShapes(sc2, sc1, fbc));
        }
    }

    SECTION("boundary conditions") {
        PeriodicBoundaryConditions pbc(100);

        SECTION("overlap") {
            sc1.translate({96.6, 50, 50}, pbc);
            sc2.translate({3.5, 50, 50}, pbc);

            CHECK(traits.overlapBetweenShapes(sc1, sc2, pbc));
            CHECK(traits.overlapBetweenShapes(sc2, sc1, pbc));
        }

        SECTION("no overlap") {
            sc1.translate({96.4, 50, 50}, pbc);
            sc2.translate({3.5, 50, 50}, pbc);

            CHECK_FALSE(traits.overlapBetweenShapes(sc1, sc2, pbc));
            CHECK_FALSE(traits.overlapBetweenShapes(sc2, sc1, pbc));
        }
    }
}

TEST_CASE("XenoColllide: spherocylinder wall overlap") {
    double l = 1.0, r = 0.5;
    XenoCollideSpherocylinderTraits traits(l, r);
    const Interaction &interaction = traits.getInteraction();

    CHECK(interaction.hasWallPart());

    SECTION("overlapping") {
        SECTION("without translation and rotation, near") {
            Shape sc({0, 0, 0}, Matrix<3, 3>::identity());
            CHECK(interaction.overlapWithWallForShape(sc, {l/2, 0, 0}, {-1, 0, 0}));
        }

        SECTION("without translation and rotation, far") {
            Shape sc({0, 0, 0}, Matrix<3, 3>::identity());
            CHECK(interaction.overlapWithWallForShape(sc, {-10*l, 0, 0}, {-1, 0, 0}));
        }

        SECTION("near") {
            Shape sc({1.1, 1.1, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI / 4));
            CHECK(interaction.overlapWithWallForShape(sc, {0, 1.5 + M_SQRT2 / 4, 0}, {0, -1, 0}));
        }

        SECTION("far") {
            Shape sc({100, 100, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI / 4));
            CHECK(interaction.overlapWithWallForShape(sc, {0, 1.5 + M_SQRT2 / 4, 0}, {0, -1, 0}));
        }
    }

    SECTION("non-overlapping") {
        SECTION("without translation and rotation, near") {
            Shape sc({0, 0, 0}, Matrix<3, 3>::identity());
            CHECK_FALSE(interaction.overlapWithWallForShape(sc, {l+r, 0, 0}, {-1, 0, 0}));
        }

        SECTION("without translation and rotation, far") {
            Shape sc({0, 0, 0}, Matrix<3, 3>::identity());
            CHECK_FALSE(interaction.overlapWithWallForShape(sc, {10*l+r, 0, 0}, {-1, 0, 0}));
        }
        SECTION("near") {
            Shape sc({0.9, 0.9, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI / 4));
            CHECK_FALSE(interaction.overlapWithWallForShape(sc, {0, 1.5 + M_SQRT2 / 4, 0}, {0, -1, 0}));
        }

        SECTION("far") {
            Shape sc({-100, -100, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI / 4));
            CHECK_FALSE(interaction.overlapWithWallForShape(sc, {0, 1.5 + M_SQRT2 / 4, 0}, {0, -1, 0}));
        }
    }
}


TEST_CASE("XenoCollide: spherocylinder basic features") {
    double l = 3, r = 2;
    XenoCollideSpherocylinderTraits traits(l, r);

    SECTION("volume") {
        CHECK(traits.getVolume() == Approx(68 * M_PI / 3));
    }

    SECTION("primary axis") {
        // primary axis X rotated 90 deg around z axis => primary axis is Y
        Shape shape({}, Matrix<3, 3>::rotation(0, 0, M_PI_2));

        CHECK_THAT(traits.getPrimaryAxis(shape), IsApproxEqual({0, 1, 0}, 1e-8));
    }

    SECTION("secondary axis") {
        // secondary axis Z rotated 90 deg around y axis => primary axis is X
        Shape shape({}, Matrix<3, 3>::rotation(0, M_PI_2, 0));

        CHECK_THAT(traits.getSecondaryAxis(shape), IsApproxEqual({1, 0, 0}, 1e-8));
    }

    SECTION("geometric origin") {
        CHECK(traits.getGeometry().getGeometricOrigin({}) == Vector<3>{0, 0, 0});
    }

    SECTION("range radius") {
        double expectedRange = l + 2*r;
        CHECK(traits.getInteraction().getRangeRadius() == expectedRange);
        CHECK(traits.getInteraction().getTotalRangeRadius() == expectedRange);
    }

    SECTION("named points") {
        CHECK(traits.getGeometry().getNamedPoint("beg", {}) == Vector<3>{-1.5, 0, 0});
        CHECK(traits.getGeometry().getNamedPoint("end", {}) == Vector<3>{1.5, 0, 0});
        CHECK(traits.getGeometry().getNamedPoint("cm", {}) == Vector<3>{0, 0, 0});
        CHECK(traits.getGeometry().getNamedPoint("o", {}) == Vector<3>{0, 0, 0});
    }
}

TEST_CASE("XenoCollide: dimer overlap") {
    // The same test as for PolysphereTraits
    XenoCollideDimerTraits traits(0.5, 1, 3);
    const Interaction &interaction = traits.getInteraction();
    PeriodicBoundaryConditions pbc(10);

    SECTION("overlap") {
        Shape shape1({1.01, 5, 5}, Matrix<3, 3>::identity());
        Shape shape2({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0));
        CHECK(interaction.overlapBetweenShapes(shape1, shape2, pbc));
    }

    SECTION("no overlap") {
        Shape shape1({0.99, 5, 5}, Matrix<3, 3>::identity());
        Shape shape2({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0));
        CHECK_FALSE(interaction.overlapBetweenShapes(shape1, shape2, pbc));
    }
}

TEST_CASE("XenoCollide: dimer ranges") {
    // The same test as for PolysphereTraits
    XenoCollideDimerTraits traits(0.5, 1, 3);
    const Interaction &interaction = traits.getInteraction();

    CHECK(interaction.getRangeRadius() == 2);
    CHECK(interaction.getTotalRangeRadius() == 8);
}