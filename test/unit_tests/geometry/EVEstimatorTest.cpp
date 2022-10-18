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
        estimator.calculate((size_t)1e+6);
        double res = estimator.getResult();
        double expect = 4.0 / 3.0 * M_PI * 8 * r * r * r;
        CHECK(std::abs(res / expect - 1) < 0.01);
    }

    SECTION("expectedError") {
        EVEstimator estimator(traits);
        estimator.calculate(0.01);
        CHECK(std::abs(estimator.getError()) < 0.01);
    }
}

TEST_CASE("EVEstimatorTest: spherocylinder") {

    double l=2.0, r = 1.0; // std::pow(3.0/(4*M_PI), 1.0/3.0);
    SpherocylinderTraits traits(l, r);
    SECTION("sample") {
        EVEstimator estimator(traits);
        estimator.calculate(Matrix<3, 3>::identity(), (size_t)1e+6);
        double res = estimator.getResult();
        double expect = M_PI*4*r*r*2*l + 4.0/3.0*M_PI*8*r*r*r;
        CHECK(std::abs(res / expect - 1) < 0.01);
    }

    SECTION("expectedError") {
        EVEstimator estimator(traits);
        estimator.calculate(Matrix<3, 3>::identity(), 0.1);
        CHECK(std::abs(estimator.getError()) < 0.1);
    }
}
/*
TEST_CASE("EVEstimator: Wedge"){
    size_t samples = 1e+4;
    double res, err;

    for (double d=0.01; d<=1.001; d+=0.01) {
        std::cout << d << "\t";
        PolysphereWedgeTraits traits(6, d, 1, 0);
        EVEstimator estimator(traits);
        estimator.calculate(Matrix<3, 3>::identity(), samples);
        res = estimator.getResult();
        err = estimator.getError();
        std::cout << res << "\t" << err << "\t";
        estimator.calculate(Matrix<3, 3>::rotation({0, 0, 1}, M_PI), samples);
        res = estimator.getResult();
        err = estimator.getError();
        std::cout << res << "\t" << err << std::endl;
    }
    CHECK(true);
}
*/