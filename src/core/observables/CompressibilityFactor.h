//
// Created by Piotr Kubala on 23/03/2021.
//

#ifndef RAMPACK_COMPRESSIBILITYFACTOR_H
#define RAMPACK_COMPRESSIBILITYFACTOR_H

#include "core/Observable.h"

class CompressibilityFactor : public Observable {
private:
    double compressibilityFactor{};

public:
    void calculate(const Packing &packing, double temperature, double pressure,
                   [[maybe_unused]] const ShapeTraits &shapeTraits) override
    {
        this->compressibilityFactor = pressure / temperature / packing.getNumberDensity();
    }

    [[nodiscard]] std::vector<std::string> getHeader() const override { return {"Z"}; }
    [[nodiscard]] std::vector<double> getValues() const override { return {this->compressibilityFactor}; }
    [[nodiscard]] std::string getName() const override { return "compressibility factor"; };
};


#endif //RAMPACK_COMPRESSIBILITYFACTOR_H
