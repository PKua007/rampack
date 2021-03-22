//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_BOXDIMENSIONS_H
#define RAMPACK_BOXDIMENSIONS_H

#include <array>

#include "core/Observable.h"

class BoxDimensions : public Observable {
private:
    std::array<double, 3> dimensions{0, 0, 0};

public:
    void calculate(const Packing &packing) override { this->dimensions = packing.getDimensions(); }
    [[nodiscard]] std::vector<std::string> getHeader() const override { return {"L_X", "L_Y", "L_Z"}; }

    [[nodiscard]] std::vector<double> getValues() const override {
        return std::vector<double>(this->dimensions.begin(), this->dimensions.end());
    }

    [[nodiscard]] std::string getName() const override { return "box dimensions"; }
};


#endif //RAMPACK_BOXDIMENSIONS_H
