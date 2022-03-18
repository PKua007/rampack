//
// Created by pkua on 18.03.2022.
//

#ifndef RAMPACK_MOVESAMPLER_H
#define RAMPACK_MOVESAMPLER_H

#include <random>
#include <vector>

#include "geometry/Matrix.h"
#include "geometry/Vector.h"


class MoveSampler {
public:
    enum class MoveType {
        TRANSLATION,
        ROTATION,
        ROTOTRANSLATION
    };

    struct MoveData {
        MoveType moveType = MoveType::ROTOTRANSLATION;
        std::size_t particleIdx{};
        Vector<3> translation{};
        Matrix<3, 3> rotation{};
    };

    virtual ~MoveSampler() = default;

    [[nodiscard]] virtual std::string getName() const = 0;

    virtual MoveData sampleMove(const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt) = 0;

    virtual bool increaseStepSize() = 0;
    virtual bool decreaseStepSize() = 0;
    [[nodiscard]] virtual std::vector<std::pair<std::string, double>> getStepSizes() const = 0;
};


#endif //RAMPACK_MOVESAMPLER_H
