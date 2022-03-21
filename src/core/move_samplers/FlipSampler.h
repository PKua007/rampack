//
// Created by pkua on 21.03.2022.
//

#ifndef RAMPACK_FLIPSAMPLER_H
#define RAMPACK_FLIPSAMPLER_H

#include "core/MoveSampler.h"


class FlipSampler : public MoveSampler {
private:
    std::size_t flipEvery{};

public:
    explicit FlipSampler(std::size_t flipEvery);

    [[nodiscard]] std::string getName() const override { return "flip"; }
    [[nodiscard]] std::size_t getNumOfRequestedMoves(std::size_t numParticles) const override;
    MoveData sampleMove(const Packing &packing, const ShapeTraits &shapeTraits,
                        const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt) override;
    bool increaseStepSize() override { return false; }
    bool decreaseStepSize() override { return false; }
    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override { return {{"flip", 0}}; }
    void setStepSize(const std::string &, double) override { }
};


#endif //RAMPACK_FLIPSAMPLER_H
