//
// Created by Piotr Kubala on 22/03/2021.
//

#ifndef RAMPACK_BOXDIMENSIONS_H
#define RAMPACK_BOXDIMENSIONS_H

#include <array>

#include "core/Observable.h"

/**
 * @brief Observable representing box dimensions (three interval values).
 */
class BoxDimensions : public Observable {
private:
    std::array<double, 3> dimensions{0, 0, 0};

public:
    void calculate(const Packing &packing, [[maybe_unused]] double temperature, [[maybe_unused]] double pressure,
                   [[maybe_unused]] const ShapeTraits &shapeTraits) override
    {
        this->dimensions = packing.getDimensions();
    }

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return {"L_X", "L_Y", "L_Z"}; }

    [[nodiscard]] std::vector<double> getIntervalValues() const override {
        return std::vector<double>(this->dimensions.begin(), this->dimensions.end());
    }

    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]]  std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "box dimensions"; }
};


#endif //RAMPACK_BOXDIMENSIONS_H
