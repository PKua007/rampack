//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_PACKINGFRACTION_H
#define RAMPACK_PACKINGFRACTION_H

#include "core/Observable.h"
#include "utils/Assertions.h"

class PackingFraction : public Observable {
private:
    double packingFraction{};

public:
    void calculate(const Packing &packing, [[maybe_unused]] double temperature, [[maybe_unused]] double pressure,
                   const ShapeTraits &shapeTraits) override
    {
        this->packingFraction = packing.getPackingFraction(shapeTraits.getVolume());
    }

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return {"theta"}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override { return {this->packingFraction}; }
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]]  std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "packing fraction"; };
};


#endif //RAMPACK_PACKINGFRACTION_H
