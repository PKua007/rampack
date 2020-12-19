//
// Created by Piotr Kubala on 17/12/2020.
//

#include <catch2/catch.hpp>

#include "core/shapes/Spherocylinder.h"
#include "core/FreeBoundaryConditions.h"
#include "core/PeriodicBoundaryConditions.h"

TEST_CASE("Spherocylinder: overlap") {
    FreeBoundaryConditions fbc;
    Spherocylinder sc1(3, 2), sc2(3, 2);

    // Cases are found visually using Mathematica. See wolfram/spheroc_test.nb
    SECTION("sphere-sphere") {
        sc1.translate({1, 2, 3}, fbc);
        sc1.rotate({  0.92669949443125, -0.3009522885099318,   0.2250683608628711,
                    0.3235062902233933,   0.943614995716347, -0.07024542721869537,
                    -0.191237358292679,  0.1379074323590797,    0.971807497858173});
        sc2.translate({-3.5, 2.9, -0.5}, fbc);
        sc2.rotate({0.1017126282787795, -0.2093565370521866,  0.972535028491077,
                    0.5191108031629524,   0.845122866944617,  0.127637431056981,
                    -0.848633322046771,  0.4918711011645504, 0.1946389081120092});

        CHECK(sc1.overlap(sc2, 0.95, fbc));
        CHECK(sc2.overlap(sc1, 0.95, fbc));
        CHECK_FALSE(sc1.overlap(sc2, 1.05, fbc));
        CHECK_FALSE(sc2.overlap(sc1, 1.05, fbc));
    }

    SECTION("sphere-cylinder") {
        sc1.translate({1, 2, 3}, fbc);
        sc1.rotate({   0.936848795202308, -0.03157560239884583,  0.34830635403497,
                    -0.03157560239884583,    0.984212198800577, 0.174153177017485,
                       -0.34830635403497,   -0.174153177017485, 0.921060994002885});
        sc2.translate({-0.5, 4, -2}, fbc);
        sc2.rotate({0.07918504528890558, 0.3698006820181268, -0.925730621823391,
                    -0.2008438095940726,  0.915521563787973,  0.348542723904781,
                      0.976417683550607, 0.1583278933673811, 0.1467677942585273});

        CHECK(sc1.overlap(sc2, 0.95, fbc));
        CHECK(sc2.overlap(sc1, 0.95, fbc));
        CHECK_FALSE(sc1.overlap(sc2, 1.05, fbc));
        CHECK_FALSE(sc2.overlap(sc1, 1.05, fbc));
    }

    SECTION("cylinder-cylinder") {
        sc1.translate({1, 2, 3}, fbc);
        sc1.rotate({  0.921060994002885,   0.1446263415347036,    0.361565853836759,
                    -0.1446263415347036,    0.989111861241777, -0.02722034689555686,
                     -0.361565853836759, -0.02722034689555686,    0.931949132761108});
        sc2.translate({-0.5, 1.7, -0.8}, fbc);
        sc2.rotate({ 0.4161880560663263, 0.5570946854720225, -0.7186327388914037,
                     -0.902545539870646, 0.1570999152673587, -0.4009129145869377,
                    -0.1104493116652922,  0.815453939865286,  0.5681864320017205});

        CHECK(sc1.overlap(sc2, 0.95, fbc));
        CHECK(sc2.overlap(sc1, 0.95, fbc));
        CHECK_FALSE(sc1.overlap(sc2, 1.05, fbc));
        CHECK_FALSE(sc2.overlap(sc1, 1.05, fbc));
    }

    SECTION("boundary conditions") {
        PeriodicBoundaryConditions pbc(100);
        sc1.translate({96.5, 50, 50}, pbc);
        sc2.translate({3.5, 50, 50}, pbc);

        CHECK(sc1.overlap(sc2, 0.95, pbc));
        CHECK(sc2.overlap(sc1, 0.95, pbc));
        CHECK_FALSE(sc1.overlap(sc2, 1.05, pbc));
        CHECK_FALSE(sc2.overlap(sc1, 1.05, pbc));
    }
}

TEST_CASE("Spherocylinder: getVolume") {
    Spherocylinder sc(3, 2);

    CHECK(sc.getVolume() == Approx(68*M_PI/3));
}

TEST_CASE("Spherocylinder: toWolfram") {
    FreeBoundaryConditions fbc;
    Spherocylinder sc(3, 2);
    sc.translate({1, 2, 3}, fbc);
    sc.rotate(Matrix<3, 3>::rotation(0, M_PI/2, 0));    // spherocylinder parallel to z axis

    CHECK(sc.toWolfram(2) == "CapsuleShape[{{2.000000, 4.000000, 7.500000},{2.000000, 4.000000, 4.500000}},2.000000]");
}