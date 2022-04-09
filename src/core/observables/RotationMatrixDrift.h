//
// Created by pkua on 09.04.2022.
//

#ifndef RAMPACK_ROTATIONMATRIXDRIFT_H
#define RAMPACK_ROTATIONMATRIXDRIFT_H

#include "core/Observable.h"


class RotationMatrixDrift : public Observable {
private:
    double frobenius2{};

public:
    void calculate(const Packing &packing, double temperature, double pressure,
                   const ShapeTraits &shapeTraits) override;

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return {"F^2"}; }
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override { return {this->frobenius2}; }
    [[nodiscard]] std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "rotation matrix drift"; }
};


#endif //RAMPACK_ROTATIONMATRIXDRIFT_H
