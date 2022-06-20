//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_PRESSURE_H
#define RAMPACK_PRESSURE_H

#include "core/Observable.h"


class Pressure : public Observable {
private:
    double pressure_{};

public:
    void calculate([[maybe_unused]] const Packing &packing, [[maybe_unused]] double temperature, double pressure,
                   [[maybe_unused]] const ShapeTraits &shapeTraits) override
    {
        this->pressure_ = pressure;
    }

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return {"p"}; }
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override { return {this->pressure_}; }
    [[nodiscard]] std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "pressure"; }
};


#endif //RAMPACK_PRESSURE_H
