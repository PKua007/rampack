//
// Created by pkua on 21.03.2022.
//

#ifndef RAMPACK_FLIPSAMPLER_H
#define RAMPACK_FLIPSAMPLER_H

#include "core/MoveSampler.h"

/**
 * @brief MoveSampler performing 180-degree rotations (flips) around molecule's secondary axis - the main axis changes
 * the sign. Internally it consists of a single move named @a flip. The group name is also @a flip.
 */
class FlipSampler : public MoveSampler {
private:
    std::size_t flipEvery{};
    Vector<3> flipAxis;
    Vector<3> geometricOrigin;
    bool isGeometricOriginZero{};

public:
    /**
     * @brief Constructs the class specifying how often to perform a flip (i.e. how many moves should be requested,
     * calculated by dividing the number of molecules by @a flipEvery).
     */
    explicit FlipSampler(std::size_t flipEvery);

    [[nodiscard]] std::string getName() const override { return "flip"; }
    [[nodiscard]] std::size_t getNumOfRequestedMoves(std::size_t numParticles) const override;
    MoveData sampleMove(const Packing &packing, const std::vector<std::size_t> &particleIdxs, std::mt19937 &mt) override;
    bool increaseStepSize() override { return false; }
    bool decreaseStepSize() override { return false; }
    [[nodiscard]] std::vector<std::pair<std::string, double>> getStepSizes() const override { return {{"flip", 0}}; }
    void setStepSize(const std::string &, double) override { }

    void setupForShapeTraits(const ShapeTraits &shapeTraits) override;
};


#endif //RAMPACK_FLIPSAMPLER_H
