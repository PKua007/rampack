//
// Created by Piotr Kubala on 23/03/2021.
//

#ifndef RAMPACK_ENERGYPERPARTICLE_H
#define RAMPACK_ENERGYPERPARTICLE_H

#include "core/Observable.h"

class EnergyPerParticle : public Observable {
private:
    double energyPerParticle{};

public:
    void calculate(const Packing &packing, [[maybe_unused]] double temperature, [[maybe_unused]] double pressure,
                   const ShapeTraits &shapeTraits) override
    {
        this->energyPerParticle = packing.getTotalEnergy(shapeTraits.getInteraction()) / packing.size();
    }

    [[nodiscard]] std::vector<std::string> getHeader() const override { return {"E"}; }
    [[nodiscard]] std::vector<double> getValues() const override { return {this->energyPerParticle}; }
    [[nodiscard]] std::string getName() const override { return "energy per particle"; };
};


#endif //RAMPACK_ENERGYPERPARTICLE_H
