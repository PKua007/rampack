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
    std::optional<ShapeGeometry::Axis> axis{};

    [[nodiscard]] std::string getSubscript() const;

public:
    /**
     * @brief Returns eigenvalues of a given @a matrix.
     */
    static std::array<double, 3> calculateEigenvalues(const Matrix<3, 3> &matrix);

    /**
     * @brief Creates the class. If @a dumpQTensor_
     *
     * @param dumpQTensor_ If @a true, the upper-triangle part of the Q-tensor will be also dumped.
     * @param axis Shape axis for which the nematic order will be computed. For backwards compatibility, if
     * @a std::nullopt is passed, it defaults to ShapeGeometry::Axis::PRIMARY. The interval header has then no
     * additional subscripts. For the explicitly specified axis, the subscript @a pa, @a sa, or @a aa is added (for,
     * respectively, ShapeGeometry::Axis::PRIMARY, ShapeGeometry::Axis::SECONDARY, and ShapeGeometry::Axis::AUXILIARY).
     */
    explicit NematicOrder(bool dumpQTensor_ = false, std::optional<ShapeGeometry::Axis> axis = std::nullopt)
        : dumpQTensor{dumpQTensor_}, axis{axis} { }

    void calculate(const Packing &packing, double temperature, double pressure,
                   const ShapeTraits &shapeTraits) override;
    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override;
    [[nodiscard]] std::vector<double> getIntervalValues() const override;
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]] std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "nematic order"; };
};


#endif //RAMPACK_NEMATICORDER_H
