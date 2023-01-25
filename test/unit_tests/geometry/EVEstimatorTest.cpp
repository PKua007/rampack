//
// Created by Michal Ciesla on 12/10/2022.
//

#include <catch2/catch.hpp>
#include <iostream>

#include "core/shapes/SphereTraits.h"
#include "geometry/EVEstimator.h"
#include "core/shapes/PolysphereWedgeTraits.h"
#include "core/shapes/SpherocylinderTraits.h"

TEST_CASE("EVEstimatorTest: sphere") {

    double r = 1.0; // std::pow(3.0/(4*M_PI), 1.0/3.0);
    SphereTraits traits(r);
    SECTION("sample") {
        EVEstimator estimator(traits);
        estimator.calculateMC(nullptr, (size_t) 1e+6);
        double res = estimator.getResult();
        double expect = 8.0; // 4.0 / 3.0 * M_PI * 8 * r * r * r;
        CHECK(std::abs(res / expect - 1) < 0.01);
    }

    SECTION("expectedError") {
        EVEstimator estimator(traits);
        estimator.calculateMC(nullptr, 0.01);
        CHECK(std::abs(estimator.getError()) < 0.01);
    }
}

TEST_CASE("EVEstimatorTest: spherocylinder") {

    double l=2.0, r = 1.0; // std::pow(3.0/(4*M_PI), 1.0/3.0);
    SpherocylinderTraits traits(l, r);
    SECTION("sampleMC") {
        EVEstimator estimator(traits);
        Matrix<3, 3> id = Matrix<3, 3>::identity();
        estimator.calculateMC(&id, (size_t) 1e+6);
        double res = estimator.getResult();
        double err = estimator.getError();
        double expect = 8; //M_PI*8*r*r(l + 4.0/3.0*r);
        CHECK(std::abs(res - expect) < 5*err);
    }

    SECTION("sampleBox") {
        EVEstimator estimator(traits);
        Matrix<3, 3> id = Matrix<3, 3>::identity();
        estimator.calculateBox(&id, 0.01);
        double res = estimator.getResult();
        double expect = 8; //M_PI*8*r*r(l + 4.0/3.0*r);
        CHECK(std::abs(res / expect - 1) < 0.01);
    }

    SECTION("expectedErrorMC") {
        EVEstimator estimator(traits);
        Matrix<3, 3> id = Matrix<3, 3>::identity();
        estimator.calculateMC(&id, 0.1);
        CHECK(std::abs(estimator.getError()) < 0.1);
    }
}
/*
TEST_CASE("EVEstimator: Wedge"){
    size_t samples = 1e+7;

    double res, err;

    for (double d=0.01; d<=1.001; d+=0.01) {
        std::cout << d << "\t" <<std::flush;
        PolysphereWedgeTraits traits(6, d, 1, 0);
        EVEstimator estimator(traits);
        Matrix<3, 3> id = Matrix<3, 3>::identity();
        estimator.calculateMC(&id, samples);
        res = estimator.getResult();
        err = estimator.getError();
        std::cout << res << "\t" << err << "\t" << std::flush;
        estimator.clear();
        Matrix<3, 3> rot = Matrix<3, 3>::rotation({0, 0, 1}, M_PI);
        estimator.calculateMC(&rot, samples);
        res = estimator.getResult();
        err = estimator.getError();
        std::cout << res << "\t" << err << std::endl << std::flush;
    }
    CHECK(true);
}
*/
