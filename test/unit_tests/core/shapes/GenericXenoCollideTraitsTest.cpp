//
// Created by Michal Ciesla on 9/4/22.
//

#include <catch2/catch.hpp>

#include "core/shapes/GenericXenoCollideTraits.h"
#include "core/FreeBoundaryConditions.h"
#include "core/PeriodicBoundaryConditions.h"

#include "utils/Exceptions.h"

#include "matchers/VectorApproxMatcher.h"
#include "geometry/xenocollide/XCBodyBuilder.h"
#include "geometry/xenocollide/XCPrimitives.h"


namespace {
    GenericXenoCollideShape create_spherocylinder(double l, double r) {
        Expects(r > 0);
        Expects(l > 0);

        XCBodyBuilder bb;
        bb.sphere(r);
        bb.move(-l/2, 0, 0);
        bb.sphere(r);
        bb.move(l/2, 0, 0);
        bb.wrap();
        auto geometry = bb.releaseCollideGeometry();

        double volume = M_PI*r*r*(4./3*r + l);

        return GenericXenoCollideShape(std::move(geometry), volume, {1, 0, 0}, {0, 0, 1}, {0, 0, 0},
                                       {{"beg", {-l / 2, 0, 0}}, {"end", {l / 2,  0, 0}}});
    }

    GenericXenoCollideShape create_dimer(double r1, double r2, double x2) {
        Expects(r1 > 0);
        Expects(r2 > 0);
        Expects(x2 > 0);

        double volume = 4*M_PI/3 * (r1*r1*r1 + r1*r2*r2);

        return GenericXenoCollideShape({{std::make_shared<XCSphere>(r1), {0, 0, 0}},
                                        {std::make_shared<XCSphere>(r2), {x2, 0, 0}}},
                                       volume, {1, 0, 0}, {0, 0, 1}, {0, 0, 0});
    }
}

TEST_CASE("GenericXenoCollideTraits: spherocylinder overlap") {
    FreeBoundaryConditions fbc;
    double l = 3, r = 2;
    GenericXenoCollideTraits traits(create_spherocylinder(l, r));
    ShapeData defaultData = traits.shapeDataForDefaultSpecies();

    Shape sc1{};
    sc1.setData(defaultData);
    Shape sc2{};
    sc2.setData(defaultData);

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

TEST_CASE("GenericXenoCollideTraits: spherocylinder wall overlap") {
    double l = 1.0, r = 0.5;
    GenericXenoCollideTraits traits(create_spherocylinder(l, r));
    ShapeData defaultData = traits.shapeDataForDefaultSpecies();
    const Interaction &interaction = traits.getInteraction();

    CHECK(interaction.hasWallPart());

    SECTION("overlapping") {
        SECTION("without translation and rotation, near") {
            Shape sc({0, 0, 0}, Matrix<3, 3>::identity(), defaultData);
            CHECK(interaction.overlapWithWallForShape(sc, {l/2, 0, 0}, {-1, 0, 0}));
        }

        SECTION("without translation and rotation, far") {
            Shape sc({0, 0, 0}, Matrix<3, 3>::identity(), defaultData);
            CHECK(interaction.overlapWithWallForShape(sc, {-10*l, 0, 0}, {-1, 0, 0}));
        }

        SECTION("near") {
            Shape sc({1.1, 1.1, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI / 4), defaultData);
            CHECK(interaction.overlapWithWallForShape(sc, {0, 1.5 + M_SQRT2 / 4, 0}, {0, -1, 0}));
        }

        SECTION("far") {
            Shape sc({100, 100, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI / 4), defaultData);
            CHECK(interaction.overlapWithWallForShape(sc, {0, 1.5 + M_SQRT2 / 4, 0}, {0, -1, 0}));
        }
    }

    SECTION("non-overlapping") {
        SECTION("without translation and rotation, near") {
            Shape sc({0, 0, 0}, Matrix<3, 3>::identity(), defaultData);
            CHECK_FALSE(interaction.overlapWithWallForShape(sc, {l+r, 0, 0}, {-1, 0, 0}));
        }

        SECTION("without translation and rotation, far") {
            Shape sc({0, 0, 0}, Matrix<3, 3>::identity(), defaultData);
            CHECK_FALSE(interaction.overlapWithWallForShape(sc, {10*l+r, 0, 0}, {-1, 0, 0}));
        }
        SECTION("near") {
            Shape sc({0.9, 0.9, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI / 4), defaultData);
            CHECK_FALSE(interaction.overlapWithWallForShape(sc, {0, 1.5 + M_SQRT2 / 4, 0}, {0, -1, 0}));
        }

        SECTION("far") {
            Shape sc({-100, -100, 5}, Matrix<3, 3>::rotation({0, 0, 1}, M_PI / 4), defaultData);
            CHECK_FALSE(interaction.overlapWithWallForShape(sc, {0, 1.5 + M_SQRT2 / 4, 0}, {0, -1, 0}));
        }
    }
}


TEST_CASE("GenericXenoCollideTraits: spherocylinder basic features") {
    double l = 3, r = 2;
    GenericXenoCollideTraits traits(create_spherocylinder(l, r));
    ShapeData defaultData = traits.shapeDataForDefaultSpecies();
    Shape defaultShape;
    defaultShape.setData(defaultData);

    SECTION("volume") {
        CHECK(traits.getVolume(defaultShape) == Approx(68 * M_PI / 3));
    }

    SECTION("primary axis") {
        // primary axis X rotated 90 deg around z axis => primary axis is Y
        Shape shape({}, Matrix<3, 3>::rotation(0, 0, M_PI_2), defaultData);

        CHECK_THAT(traits.getPrimaryAxis(shape), IsApproxEqual({0, 1, 0}, 1e-8));
    }

    SECTION("secondary axis") {
        // secondary axis Z rotated 90 deg around y axis => primary axis is X
        Shape shape({}, Matrix<3, 3>::rotation(0, M_PI_2, 0), defaultData);

        CHECK_THAT(traits.getSecondaryAxis(shape), IsApproxEqual({1, 0, 0}, 1e-8));
    }

    SECTION("geometric origin") {
        CHECK(traits.getGeometry().getGeometricOrigin(defaultShape) == Vector<3>{0, 0, 0});
    }

    SECTION("named points") {
        CHECK(traits.getGeometry().getNamedPointForShape("beg", defaultShape) == Vector<3>{-1.5, 0, 0});
        CHECK(traits.getGeometry().getNamedPointForShape("end", defaultShape) == Vector<3>{1.5, 0, 0});
        CHECK(traits.getGeometry().getNamedPointForShape("o", defaultShape) == Vector<3>{0, 0, 0});
    }
}

TEST_CASE("GenericXenoCollideTraits: dimer overlap") {
    // The same test as for PolysphereTraits
    GenericXenoCollideTraits traits(create_dimer(0.5, 1, 3));
    ShapeData defaultData = traits.shapeDataForDefaultSpecies();
    const Interaction &interaction = traits.getInteraction();
    PeriodicBoundaryConditions pbc(10);

    SECTION("overlap") {
        Shape shape1({1.01, 5, 5}, Matrix<3, 3>::identity(), defaultData);
        Shape shape2({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0), defaultData);
        CHECK(interaction.overlapBetweenShapes(shape1, shape2, pbc));
    }

    SECTION("no overlap") {
        Shape shape1({0.99, 5, 5}, Matrix<3, 3>::identity(), defaultData);
        Shape shape2({9, 5, 5}, Matrix<3, 3>::rotation(0, M_PI, 0), defaultData);
        CHECK_FALSE(interaction.overlapBetweenShapes(shape1, shape2, pbc));
    }
}

TEST_CASE("GenericXenoCollideTraits: polydisperse overlap") {
    GenericXenoCollideTraits traits;
    auto dimerData = traits.addSpecies("dimer", create_dimer(0.5, 0.5, 1));
    auto scData = traits.addSpecies("spherocylinder", create_spherocylinder(1, 0.5));
    // Dimer lies along x-axis, spherocylinder along y-axis; spherocylinder's cup is tangent to both dimer beads
    Shape dimerShape({}, Matrix<3, 3>::identity(), dimerData);
    Shape scShape({0.5, 0.5*(1 + std::sqrt(3.)), 0}, Matrix<3, 3>::rotation(0, 0, M_PI/2), scData);
    FreeBoundaryConditions fbc;

    SECTION("overlap") {
        scShape.translate({0, -0.1, 0}, fbc);
        CHECK(traits.overlapBetweenShapes(dimerShape, scShape, fbc));
    }

    SECTION("no overlap") {
        scShape.translate({0, 0.1, 0}, fbc);
        CHECK_FALSE(traits.overlapBetweenShapes(dimerShape, scShape, fbc));
    }
}

TEST_CASE("GenericXenoCollideTraits: range radius") {
    SECTION("spherocylinder (single interaction centre)") {
        GenericXenoCollideTraits traits(create_spherocylinder(3, 2));
        ShapeData defaultData = traits.shapeDataForDefaultSpecies();

        CHECK(traits.getInteraction().getRangeRadius(defaultData.raw()) == 7);
        CHECK(traits.getInteraction().getTotalRangeRadius(defaultData.raw()) == 7);
    }

    SECTION("dimer (multiple interaction centres)") {
        GenericXenoCollideTraits traits(create_dimer(1, 0.5, 1));
        ShapeData defaultData = traits.shapeDataForDefaultSpecies();

        CHECK(traits.getInteraction().getRangeRadius(defaultData.raw()) == 2);
        CHECK(traits.getInteraction().getTotalRangeRadius(defaultData.raw()) == 3);
    }
}

TEST_CASE("GenericXenoCollideTraits: consistent interaction centres") {
    SECTION("single centre") {
        GenericXenoCollideTraits traits;
        traits.addSpecies("spheroc1", create_spherocylinder(1, 0.5));
        traits.addSpecies("spheroc2", create_spherocylinder(2, 0.5));

        CHECK_FALSE(traits.isMulticentre());
        CHECK(traits.getSpecies(0).getInteractionCentres().empty());
        CHECK(traits.getSpecies(1).getInteractionCentres().empty());
    }

    SECTION("multicentre pre-promotion") {
        GenericXenoCollideTraits traits;
        traits.addSpecies("dimer", create_dimer(0.5, 0.5, 1));
        traits.addSpecies("spheroc", create_spherocylinder(2, 0.5));

        CHECK(traits.isMulticentre());
        CHECK(traits.getSpecies(0).getInteractionCentres() == std::vector<Vector<3>>{{0, 0, 0}, {1, 0, 0}});
        CHECK(traits.getSpecies(1).getInteractionCentres() == std::vector<Vector<3>>{{0, 0, 0}});
    }

    SECTION("multicentre post-promotion") {
        GenericXenoCollideTraits traits;
        traits.addSpecies("spheroc", create_spherocylinder(2, 0.5));
        traits.addSpecies("dimer", create_dimer(0.5, 0.5, 1));

        CHECK(traits.isMulticentre());
        CHECK(traits.getSpecies(0).getInteractionCentres() == std::vector<Vector<3>>{{0, 0, 0}});
        CHECK(traits.getSpecies(1).getInteractionCentres() == std::vector<Vector<3>>{{0, 0, 0}, {1, 0, 0}});
    }
}

TEST_CASE("GenericXenoCollideTraits: isConvex") {
    SECTION("GenericXenoCollideShape logic") {
        SECTION("single center: convex by default") {
            GenericXenoCollideShape spheroc = create_spherocylinder(1, 0.5);

            CHECK(spheroc.isConvex());
        }

        SECTION("multiple centers: concave by default") {
            GenericXenoCollideShape dimer = create_dimer(0.5, 0.5, 1);

            CHECK_FALSE(dimer.isConvex());
        }

        SECTION("multiple centers: forced convex") {
            GenericXenoCollideShape sphereInSphere({
                {std::make_shared<XCSphere>(0.5), {0, 0, 0}},
                {std::make_shared<XCSphere>(0.25), {0, 0, 0}}
            }, 1, std::nullopt, std::nullopt, {0, 0, 0}, {}, true);

            CHECK(sphereInSphere.isConvex());
        }
    }

    SECTION("combining multiple species") {
        SECTION("convex + convex = convex") {
            GenericXenoCollideTraits traits;
            traits.addSpecies("spheroc1", create_spherocylinder(1, 1));
            traits.addSpecies("spheroc2", create_spherocylinder(2, 1));

            CHECK(traits.isConvex());
        }

        SECTION("convex + concave = concave") {
            GenericXenoCollideTraits traits;
            traits.addSpecies("spheroc", create_spherocylinder(1, 1));
            traits.addSpecies("dimer", create_dimer(0.5, 0.5, 1));

            CHECK_FALSE(traits.isConvex());
        }
    }
}