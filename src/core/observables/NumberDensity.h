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
    void calculate(const Packing &packing) override { this->numberDensity = packing.getNumberDensity(); }
    [[nodiscard]] std::vector<std::string> getHeader() const override { return {"rho"}; }
    [[nodiscard]] std::vector<double> getValues() const override { return {this->numberDensity}; }
    [[nodiscard]] std::string getName() const override { return "number density"; };
};


#endif //RAMPACK_NUMBERDENSITY_H
