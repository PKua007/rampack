//
// Created by pkua on 21.03.2022.
//

#ifndef RAMPACK_ROTATIONSAMPLER_H
#define RAMPACK_ROTATIONSAMPLER_H

#include "core/MoveSampler.h"


class RotationSampler : public MoveSampler {
private:
    double rotationStepSize{};

public:
    explicit RotationSampler(double rotationStepSize);

    [[nodiscard]] std::string getName() const override { return "rotation"; }

    [[nodiscard]] std::size_t getNumOfRequestedMoves(std::size_t numParticles) const override { return numParticles; }

    MoveData sampleMove(const Packing &packing, const ShapeTraits &shapeTraits,
                        const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt) override;
    bool increaseStepSize() override;
    bool decreaseStepSize() override;

    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override;

    void setStepSize(const std::string &stepName, double stepSize) override;
};


#endif //RAMPACK_ROTATIONSAMPLER_H
