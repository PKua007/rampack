//
// Created by pkua on 04.04.2022.
//

#include <catch2/catch.hpp>

#include "matchers/MatrixApproxMatcher.h"

#include "geometry/EulerAngles.h"


TEST_CASE("EulerAngles: non-problematic") {
    double ax = M_PI/4;
    double ay = M_PI/3;
    double az = 7*M_PI/6;
    Matrix<3, 3> rotation = Matrix<3, 3>::rotation(ax, ay, az);

    EulerAngles angles(rotation);

    Matrix<3, 3> reconstructed1 = Matrix<3, 3>::rotation(angles.first[0], angles.first[1], angles.first[2]);
    Matrix<3, 3> reconstructed2 = Matrix<3, 3>::rotation(angles.second[0], angles.second[1], angles.second[2]);

    CHECK_THAT(reconstructed1, IsApproxEqual(rotation, 1e-14));
    CHECK_THAT(reconstructed2, IsApproxEqual(rotation, 1e-14));
    CHECK_FALSE(angles.hasGimbalLock());
}

TEST_CASE("EulerAngles: gimbal lock") {
    double ax = M_PI/4;
    double ay = GENERATE(-M_PI/2, M_PI/2);
    double az = 7*M_PI/6;
    Matrix<3, 3> rotation = Matrix<3, 3>::rotation(ax, ay, az);

    EulerAngles angles(rotation);

    Matrix<3, 3> reconstructed1 = Matrix<3, 3>::rotation(angles.first[0], angles.first[1], angles.first[2]);
    Matrix<3, 3> reconstructed2 = Matrix<3, 3>::rotation(angles.second[0], angles.second[1], angles.second[2]);

    CHECK_THAT(reconstructed1, IsApproxEqual(rotation, 1e-11));
    CHECK_THAT(reconstructed2, IsApproxEqual(rotation, 1e-11));
    CHECK(angles.hasGimbalLock());
}

TEST_CASE("EulerAngles: approximate gimbal lock") {
    double ax = M_PI/4;
    double ay = GENERATE(-M_PI/2 + 1e-9, M_PI/2 - 1e-9);
    double az = 7*M_PI/6;
    Matrix<3, 3> rotation = Matrix<3, 3>::rotation(ax, ay, az);

    EulerAngles angles(rotation);

    Matrix<3, 3> reconstructed1 = Matrix<3, 3>::rotation(angles.first[0], angles.first[1], angles.first[2]);
    Matrix<3, 3> reconstructed2 = Matrix<3, 3>::rotation(angles.second[0], angles.second[1], angles.second[2]);

    CHECK_THAT(reconstructed1, IsApproxEqual(rotation, 1e-9));
    CHECK_THAT(reconstructed2, IsApproxEqual(rotation, 1e-9));
    CHECK(angles.hasGimbalLock());
}

TEST_CASE("EulerAngles: just barely not gimbal lock") {
    double ax = M_PI/4;
    double ay = GENERATE(-M_PI/2 + 1e-7, M_PI/2 - 1e-7);
    double az = 7*M_PI/6;
    Matrix<3, 3> rotation = Matrix<3, 3>::rotation(ax, ay, az);

    EulerAngles angles(rotation);

    Matrix<3, 3> reconstructed1 = Matrix<3, 3>::rotation(angles.first[0], angles.first[1], angles.first[2]);
    Matrix<3, 3> reconstructed2 = Matrix<3, 3>::rotation(angles.second[0], angles.second[1], angles.second[2]);

    CHECK_THAT(reconstructed1, IsApproxEqual(rotation, 1e-8));
    CHECK_THAT(reconstructed2, IsApproxEqual(rotation, 1e-8));
    CHECK_FALSE(angles.hasGimbalLock());
}

TEST_CASE("EulerAngles: incorrect matrix") {
    CHECK_THROWS(EulerAngles{
        Matrix<3, 3>::rotation(M_PI/4, M_PI/3, 7*M_PI/6)
        +
        Matrix<3, 3>{0, 0, 1e-8,
                     0, 0, 0,
                     0, 0, 0}
    });

    CHECK_THROWS(EulerAngles{
        Matrix<3, 3>{(1 + 1e-8), 0,            0,
                              0, 1,            0,
                              0, 0, 1/(1 + 1e-8)}
    });
}