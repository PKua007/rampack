//
// Created by pkua on 20.06.22.
//

#ifndef RAMPACK_TEMPERATURE_H
#define RAMPACK_TEMPERATURE_H

#include "core/Observable.h"


class Temperature : public Observable {
private:
    double temperature_{};

public:
    void calculate([[maybe_unused]] const Packing &packing, double temperature, [[maybe_unused]] double pressure,
                   [[maybe_unused]] const ShapeTraits &shapeTraits) override
    {
        this->temperature_ = temperature;
    }

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return {"T"}; }
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override { return {this->temperature_}; }
    [[nodiscard]] std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "temperature"; }
};


#endif //RAMPACK_TEMPERATURE_H
