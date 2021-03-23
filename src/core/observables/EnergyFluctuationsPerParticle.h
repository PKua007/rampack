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
                   const Interaction &interaction) override
    {
        this->energyFluctuationsPerParticle = packing.getParticleEnergyFluctuations(interaction);
    }

    [[nodiscard]] std::vector<std::string> getHeader() const override { return {"varE"}; }
    [[nodiscard]] std::vector<double> getValues() const override { return {this->energyFluctuationsPerParticle}; }
    [[nodiscard]] std::string getName() const override { return "energy fluctuations per particle"; };
};

#endif //RAMPACK_ENERGYFLUCTUATIONSPERPARTICLE_H
