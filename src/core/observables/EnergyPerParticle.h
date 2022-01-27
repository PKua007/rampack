//
// Created by Piotr Kubala on 23/03/2021.
//

#ifndef RAMPACK_ENERGYPERPARTICLE_H
#define RAMPACK_ENERGYPERPARTICLE_H

#include "core/Observable.h"

/**
 * @brief Energy per particle interval observable.
 */
class EnergyPerParticle : public Observable {
private:
    double energyPerParticle{};

public:
    void calculate(const Packing &packing, [[maybe_unused]] double temperature, [[maybe_unused]] double pressure,
                   const ShapeTraits &shapeTraits) override
    {
        this->energyPerParticle = packing.getTotalEnergy(shapeTraits.getInteraction()) / packing.size();
    }

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return {"E"}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override { return {this->energyPerParticle}; }
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]]  std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "energy per particle"; };
};


#endif //RAMPACK_ENERGYPERPARTICLE_H
