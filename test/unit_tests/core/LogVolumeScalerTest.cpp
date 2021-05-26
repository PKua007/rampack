//
// Created by Piotr Kubala on 26/05/2021.
//

#include <random>
#include <catch2/catch.hpp>

#include "core/volume_scalers/LogVolumeScaler.h"

TEST_CASE("LogVolumeScaler") {
    std::mt19937 mt(1234);

    SECTION("scale together") {
        SECTION("isotropic") {
            LogVolumeScaler volumeScaler(VolumeScaler::ScalingDirection::ISOTROPIC, true);

            auto scaleFactors = volumeScaler.sampleScalingFactors({1, 1, 1}, 0.1, mt);

            CHECK(scaleFactors[0] != 1.);
            CHECK(scaleFactors[0] == scaleFactors[1]);
            CHECK(scaleFactors[1] == scaleFactors[2]);
        }

        SECTION("anisotropic X") {
            LogVolumeScaler volumeScaler(VolumeScaler::ScalingDirection::ANISOTROPIC_X, true);

            auto scaleFactors = volumeScaler.sampleScalingFactors({1, 1, 1}, 0.1, mt);

            CHECK(scaleFactors[0] != 1.); CHECK(scaleFactors[1] != 1.);
            CHECK(scaleFactors[0] != scaleFactors[1]);
            CHECK(scaleFactors[1] == scaleFactors[2]);
        }

        SECTION("anisotropic Y") {
            LogVolumeScaler volumeScaler(VolumeScaler::ScalingDirection::ANISOTROPIC_Y, true);

            auto scaleFactors = volumeScaler.sampleScalingFactors({1, 1, 1}, 0.1, mt);

            CHECK(scaleFactors[1] != 1.); CHECK(scaleFactors[2] != 1.);
            CHECK(scaleFactors[1] != scaleFactors[2]);
            CHECK(scaleFactors[2] == scaleFactors[0]);
        }

        SECTION("anisotropic Z") {
            LogVolumeScaler volumeScaler(VolumeScaler::ScalingDirection::ANISOTROPIC_Z, true);

            auto scaleFactors = volumeScaler.sampleScalingFactors({1, 1, 1}, 0.1, mt);

            CHECK(scaleFactors[2] != 1.); CHECK(scaleFactors[0] != 1.);
            CHECK(scaleFactors[2] != scaleFactors[0]);
            CHECK(scaleFactors[0] == scaleFactors[1]);
        }

        SECTION("anisotropic XYZ") {
            LogVolumeScaler volumeScaler(VolumeScaler::ScalingDirection::ANISOTROPIC_XYZ, true);

            auto scaleFactors = volumeScaler.sampleScalingFactors({1, 1, 1}, 0.1, mt);

            CHECK(scaleFactors[0] != 1.); CHECK(scaleFactors[1] != 1.); CHECK(scaleFactors[2] != 1.);
            CHECK(scaleFactors[0] != scaleFactors[1]);
            CHECK(scaleFactors[1] != scaleFactors[2]);
            CHECK(scaleFactors[2] != scaleFactors[0]);
        }
    }

    SECTION("scale separately") {
        SECTION("isotropic") {
            LogVolumeScaler volumeScaler(VolumeScaler::ScalingDirection::ISOTROPIC, false);

            auto scaleFactors = volumeScaler.sampleScalingFactors({1, 1, 1}, 0.1, mt);

            CHECK(scaleFactors[0] != 1.);
            CHECK(scaleFactors[0] == scaleFactors[1]);
            CHECK(scaleFactors[1] == scaleFactors[2]);
        }

        SECTION("anisotropic X") {
            LogVolumeScaler volumeScaler(VolumeScaler::ScalingDirection::ANISOTROPIC_X, false);

            auto scaleFactors = volumeScaler.sampleScalingFactors({1, 1, 1}, 0.1, mt);

            CHECK((   (scaleFactors[0] == 1. && scaleFactors[1] != 1. && scaleFactors[2] != 1.)
                   || (scaleFactors[0] != 1. && scaleFactors[1] == 1. && scaleFactors[2] == 1.)   ));
            CHECK(scaleFactors[1] == scaleFactors[2]);
        }

        SECTION("anisotropic Y") {
            LogVolumeScaler volumeScaler(VolumeScaler::ScalingDirection::ANISOTROPIC_Y, false);

            auto scaleFactors = volumeScaler.sampleScalingFactors({1, 1, 1}, 0.1, mt);

            CHECK((   (scaleFactors[0] != 1. && scaleFactors[1] == 1. && scaleFactors[2] != 1.)
                   || (scaleFactors[0] == 1. && scaleFactors[1] != 1. && scaleFactors[2] == 1.)   ));
            CHECK(scaleFactors[2] == scaleFactors[0]);
        }

        SECTION("anisotropic Z") {
            LogVolumeScaler volumeScaler(VolumeScaler::ScalingDirection::ANISOTROPIC_Z, false);

            auto scaleFactors = volumeScaler.sampleScalingFactors({1, 1, 1}, 0.1, mt);

            CHECK((   (scaleFactors[0] != 1. && scaleFactors[1] != 1. && scaleFactors[2] == 1.)
                   || (scaleFactors[0] == 1. && scaleFactors[1] == 1. && scaleFactors[2] != 1.)   ));
            CHECK(scaleFactors[0] == scaleFactors[1]);
        }

        SECTION("anisotropic XYZ") {
            LogVolumeScaler volumeScaler(VolumeScaler::ScalingDirection::ANISOTROPIC_XYZ, false);

            auto scaleFactors = volumeScaler.sampleScalingFactors({1, 1, 1}, 0.1, mt);

            CHECK((   (scaleFactors[0] != 1. && scaleFactors[1] == 1. && scaleFactors[2] == 1.)
                   || (scaleFactors[0] == 1. && scaleFactors[1] != 1. && scaleFactors[2] == 1.)
                   || (scaleFactors[0] == 1. && scaleFactors[1] == 1. && scaleFactors[2] != 1.)   ));
        }
    }
}