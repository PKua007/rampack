//
// Created by Piotr Kubala on 23/03/2021.
//

#ifndef RAMPACK_NEMATICORDER_H
#define RAMPACK_NEMATICORDER_H

#include <array>

#include "core/Observable.h"

/**
 * @brief P2 nematic order interval observable. P2 is given by highest-magnitude eigenvalue of the Q tensor.
 */
class NematicOrder : public Observable {
private:
    bool dumpQTensor{};
    Matrix<3, 3> QTensor;
    double P2{};
    ShapeGeometry::Axis axis{};

public:
    /**
     * @brief Returns eigenvalues of a given @a matrix.
     */
    static std::array<double, 3> calculateEigenvalues(const Matrix<3, 3> &matrix);

    /**
     * @brief Creates the class. If @a dumpQTensor_ is @a true, whole Q tensor (upper-triangle part) will be also
     * dumped.
     */
    explicit NematicOrder(bool dumpQTensor_ = false, ShapeGeometry::Axis axis = ShapeGeometry::Axis::PRIMARY)
        : dumpQTensor{dumpQTensor_}, axis{axis} { }

    void calculate(const Packing &packing, double temperature, double pressure,
                   const ShapeTraits &shapeTraits) override;
    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override;
    [[nodiscard]] std::vector<double> getIntervalValues() const override;
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]]  std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "nematic order"; };
};


#endif //RAMPACK_NEMATICORDER_H
