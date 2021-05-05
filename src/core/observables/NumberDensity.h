//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_NUMBERDENSITY_H
#define RAMPACK_NUMBERDENSITY_H

#include "core/Observable.h"

class NumberDensity : public Observable {
private:
    double numberDensity{};

public:
    void calculate(const Packing &packing, [[maybe_unused]] double temperature, [[maybe_unused]] double pressure,
                   [[maybe_unused]] const ShapeTraits &shapeTraits) override
    {
        this->numberDensity = packing.getNumberDensity();
    }

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return {"rho"}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override { return {this->numberDensity}; }
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]]  std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "number density"; };
};


#endif //RAMPACK_NUMBERDENSITY_H
