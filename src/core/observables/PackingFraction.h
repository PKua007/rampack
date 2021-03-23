//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_PACKINGFRACTION_H
#define RAMPACK_PACKINGFRACTION_H

#include "core/Observable.h"
#include "utils/Assertions.h"

class PackingFraction : public Observable {
private:
    double shapeVolume{};
    double packingFraction{};

public:
    explicit PackingFraction(double shapeVolume) : shapeVolume{shapeVolume} {
        Expects(this->shapeVolume > 0);
    }

    void calculate(const Packing &packing, [[maybe_unused]] double temperature, [[maybe_unused]] double pressure,
                   [[maybe_unused]] const Interaction &interaction) override
    {
        this->packingFraction = packing.getPackingFraction(shapeVolume);
    }

    [[nodiscard]] std::vector<std::string> getHeader() const override { return {"theta"}; }
    [[nodiscard]] std::vector<double> getValues() const override { return {this->packingFraction}; }
    [[nodiscard]] std::string getName() const override { return "packing fraction"; };
};


#endif //RAMPACK_PACKINGFRACTION_H
