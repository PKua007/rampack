//
// Created by pkua on 28.06.22.
//

#include <catch2/catch.hpp>

#include "mocks/MockScalingFactorSampler.h"

#include "core/volume_scalers/AnisotropicVolumeScaler.h"


TEST_CASE("AnisotropicVolumeScaler") {
    using trompeloeil::_;
    using ba = std::array<bool, 3>;
    using da = std::array<double, 3>;

    const auto &X = AnisotropicVolumeScaler::X;
    const auto &Y = AnisotropicVolumeScaler::Y;
    const auto &Z = AnisotropicVolumeScaler::Z;

    auto factorSampler = std::make_unique<MockScalingFactorSampler>();
    std::mt19937 mt;

    SECTION("not independent") {
        SECTION("isotropic") {
            ALLOW_CALL(*factorSampler, sampleFactors(ba{true, true, true}, da{1, 2, 3}, 4, _)).RETURN(da{5, 6, 7});

            AnisotropicVolumeScaler scaler(std::move(factorSampler), X & Y & Z, false);

            CHECK(scaler.sampleScalingFactors({1, 2, 3}, 4, mt) == da{5, 6, 7});
        };

        SECTION("anisotropic x") {
            ALLOW_CALL(*factorSampler, sampleFactors(ba{true, false, false}, da{1, 2, 3}, 4, _)).RETURN(da{5, 6, 7});
            ALLOW_CALL(*factorSampler, sampleFactors(ba{false, true, true}, da{1, 2, 3}, 4, _)).RETURN(da{8, 9, 10});

            AnisotropicVolumeScaler scaler(std::move(factorSampler), X | (Y & Z), false);

            CHECK(scaler.sampleScalingFactors({1, 2, 3}, 4, mt) == da{40, 54, 70});
        };

        SECTION("anisotropic xyz") {
            ALLOW_CALL(*factorSampler, sampleFactors(ba{true, false, false}, da{1, 2, 3}, 4, _)).RETURN(da{5, 6, 7});
            ALLOW_CALL(*factorSampler, sampleFactors(ba{false, true, false}, da{1, 2, 3}, 4, _)).RETURN(da{8, 9, 10});
            ALLOW_CALL(*factorSampler, sampleFactors(ba{false, false, true}, da{1, 2, 3}, 4, _)).RETURN(da{1, 2, 3});

            AnisotropicVolumeScaler scaler(std::move(factorSampler), X | Y | Z, false);

            CHECK(scaler.sampleScalingFactors({1, 2, 3}, 4, mt) == da{40, 108, 210});
        };
    }

    SECTION("independent") {
        SECTION("anisotropic x") {
            ALLOW_CALL(*factorSampler, sampleFactors(ba{true, false, false}, da{1, 2, 3}, 4, _)).RETURN(da{5, 6, 7});
            ALLOW_CALL(*factorSampler, sampleFactors(ba{false, true, true}, da{1, 2, 3}, 4, _)).RETURN(da{8, 9, 10});

            AnisotropicVolumeScaler scaler(std::move(factorSampler), X | (Y & Z), true);

            auto factors = scaler.sampleScalingFactors({1, 2, 3}, 4, mt);
            CHECK((factors == da{5, 6, 7} || factors == da{8, 9, 10}));
        };

        SECTION("anisotropic xyz") {
            ALLOW_CALL(*factorSampler, sampleFactors(ba{true, false, false}, da{1, 2, 3}, 4, _)).RETURN(da{5, 6, 7});
            ALLOW_CALL(*factorSampler, sampleFactors(ba{false, true, false}, da{1, 2, 3}, 4, _)).RETURN(da{8, 9, 10});
            ALLOW_CALL(*factorSampler, sampleFactors(ba{false, false, true}, da{1, 2, 3}, 4, _)).RETURN(da{1, 2, 3});

            AnisotropicVolumeScaler scaler(std::move(factorSampler), X | Y | Z, true);

            auto factors = scaler.sampleScalingFactors({1, 2, 3}, 4, mt);
            CHECK((factors == da{5, 6, 7} || factors == da{8, 9, 10} || factors == da{1, 2, 3}));
        };
    }
}