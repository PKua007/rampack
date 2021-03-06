//
// Created by Piotr Kubala on 23/03/2021.
//

#ifndef RAMPACK_NEMATICORDER_H
#define RAMPACK_NEMATICORDER_H

#include "core/Observable.h"

class NematicOrder : public Observable {
private:
    static double calculateHighestEigenvalue(const Matrix<3, 3> &tensor);

    double P2;

public:
    void calculate(const Packing &packing, double temperature, double pressure,
                   const ShapeTraits &shapeTraits) override;
    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override { return {"P2"}; };
    [[nodiscard]] std::vector<double> getIntervalValues() const override { return {this->P2}; };
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]]  std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "nematic order"; };
};


#endif //RAMPACK_NEMATICORDER_H
