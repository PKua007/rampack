//
// Created by Piotr Kubala on 23/03/2021.
//

#ifndef RAMPACK_ENERGYFLUCTUATIONSPERPARTICLE_H
#define RAMPACK_ENERGYFLUCTUATIONSPERPARTICLE_H

#include "core/Observable.h"

class EnergyFluctuationsPerParticle : public Observable {
private:
    double energyFluctuationsPerParticle{};

public:
    void calculate(const Packing &packing, [[maybe_unused]] double temperature, [[maybe_unused]] double pressure,
                   const ShapeTraits &shapeTraits) override
    {
        this->energyFluctuationsPerParticle = packing.getParticleEnergyFluctuations(shapeTraits.getInteraction());
    }

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return {"varE"}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override { return {this->energyFluctuationsPerParticle}; }
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]]  std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "energy fluctuations per particle"; };
};

#endif //RAMPACK_ENERGYFLUCTUATIONSPERPARTICLE_H
