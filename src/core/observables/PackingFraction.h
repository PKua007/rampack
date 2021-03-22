//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_PACKINGFRACTION_H
#define RAMPACK_PACKINGFRACTION_H

#include "core/Observable.h"

class PackingFraction : public Observable {
private:
    double packingFraction{};

public:
    void calculate(const Packing &packing) override { this->packingFraction = packing.getPackingFraction(); }
    [[nodiscard]] std::vector<std::string> getHeader() const override { return {"theta"}; }
    [[nodiscard]] std::vector<double> getValues() const override { return {this->packingFraction}; }
    [[nodiscard]] std::string getName() const override { return "packing fraction"; };
};


#endif //RAMPACK_PACKINGFRACTION_H
