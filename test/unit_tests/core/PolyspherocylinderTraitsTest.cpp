//
// Created by Piotr Kubala on 27/04/2021.
//

#include <catch2/catch.hpp>

#include "matchers/VectorApproxMatcher.h"

#include "core/shapes/PolyspherocylinderTraits.h"
#include "core/FreeBoundaryConditions.h"
#include "core/PeriodicBoundaryConditions.h"

using Data = PolyspherocylinderTraits::SpherocylinderData;

TEST_CASE("PolyspherocylinderTraits") {
    // It looks like this (one bigger and one smaller sticking from its top - radius 1 and radius 0.5)
    //      -1  0  1  x
    //  z    |  |  |
    //         ###
    //  3 -    #C#
    //  2 -  ###O###
    //  1 -  ###X###
    //  0 -  ###O###
    // -1 -  ###C###
    // -2 -  #######
    //
    // O - middles of spherocylinders
    // C - cap centres of spherocylinders (non common)
    // X - common cap of both spherocylinders

    PolyspherocylinderTraits traits({Data{{0, 0, 0}, {0, 0, 1}, 1},
                                     Data{{0, 0, 2}, {0, 0, 1}, 0.5}},
                                    {0, 0, 1},
                                    false);

    SECTION("hard interactions") {
        const Interaction &interaction = traits.getInteraction();

        SECTION("meta info") {
            CHECK_FALSE(interaction.hasSoftPart());
            CHECK(interaction.hasHardPart());
            CHECK(interaction.getRangeRadius() == Approx(4));
            std::vector<Vector<3>> expectedCentres = {{0, 0, 0},
                                                      {0, 0, 2}};
            CHECK_THAT(interaction.getInteractionCentres(), Catch::UnorderedEquals(expectedCentres));
        }

        SECTION("overlap") {
            Shape shape1({0, 0, 0}, Matrix<3, 3>::rotation(0, M_PI / 2, 0));
            Shape shape2({4.5, 0, 0});
            FreeBoundaryConditions fbc;
            SECTION("true") {
                shape1.translate({0.00001, 0, 0}, fbc);
                CHECK(interaction.overlapBetweenShapes(shape1, shape2, fbc));
            }

            SECTION("false") {
                shape1.translate({-0.00001, 0, 0}, fbc);
                CHECK_FALSE(interaction.overlapBetweenShapes(shape1, shape2, fbc));
            }
        }
    }

    SECTION("toWolfram") {
        const ShapePrinter &printer = traits.getPrinter();
        Shape shape({-1, 0, 0}, Matrix<3, 3>::rotation(0, M_PI / 2, 0));

        CHECK(printer.toWolfram(shape)
              == "{Tube[{{0.000000, 0.000000, 0.000000},{-2.000000, 0.000000, -0.000000}},1.000000]"
                 ",Tube[{{2.000000, 0.000000, 0.000000},{0.000000, 0.000000, 0.000000}},0.500000]}");
    }

    SECTION("primary axis") {
        // primary axis Z rotated 90 deg around Y axis => primary axis is X
        Shape shape({}, Matrix<3, 3>::rotation(0, M_PI / 2, 0));
        CHECK_THAT(traits.getPrimaryAxis(shape), IsApproxEqual({1, 0, 0}, 1e-8));
    }
}

TEST_CASE("PolyspherocylinderTraits: tests from SpherocylinderTraits") {
    // There ase test cases shamelessly copied from SpherocylinderTraitsTest.cpp used to evaluate more thoroughly
    // the intersection criterion

    FreeBoundaryConditions fbc;
    PolyspherocylinderTraits traits({Data{{0, 0, 0}, {1.5, 0, 0}, 2}}, {1, 0, 0}, false);
    Shape sc1{}, sc2{};

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

TEST_CASE("PolyspherocylnderTraits: mass centre normalization") {
    // First spherocylinder has 2 times bigger volume than the second
    PolyspherocylinderTraits traits({Data{{0, 0, 0}, {0, 0, 1}, 1},
                                     Data{{0, 0, 6}, {0, 0, 3}, 0.5}},
                                    {0, 0, 1},
                                    true);

    CHECK(traits.getSpherocylinderData() == std::vector<Data>{Data{{0, 0, -2}, {0, 0, 1}, 1},
                                                              Data{{0, 0, 4}, {0, 0, 3}, 0.5}});
}