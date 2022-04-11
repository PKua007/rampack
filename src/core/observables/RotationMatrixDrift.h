//
// Created by pkua on 09.04.2022.
//

#ifndef RAMPACK_ROTATIONMATRIXDRIFT_H
#define RAMPACK_ROTATIONMATRIXDRIFT_H

#include "core/Observable.h"


/**
 * @brief A mean squared Frobenius norm of deviation of shape orientations from perfect rotation matrices.
 * @details More precisely, for each orientation \f$ R \f$ of a molecule, \f$ M = R R^T - 1 \f$ is calculated,
 * and the squared Frobenius norm \f$ ||M||^2 = \sum_{i,j} M_{ij}^2 \f$ is averaged over all molecules in the system.
 * This observable is useful to control the drift of numerical error of shape orientations during the simulation.
 */
class RotationMatrixDrift : public Observable {
private:
    double frobenius2{};
    double frobenius2max{};
    double frobenius2min{};

public:
    /**
     * @brief Calculates average squared Frobenius norm deviation of orientation in a given @a packing (rest of
     * parameters are ignored).
     */
    void calculate(const Packing &packing, double temperature, double pressure,
                   const ShapeTraits &shapeTraits) override;

    [[nodiscard]] std::vector<std::string> getIntervalHeader() const override {
        return {"F^2", "min(F^2)", "max(F^2)"};
    }
    [[nodiscard]] std::vector<std::string> getNominalHeader() const override { return {}; }
    [[nodiscard]] std::vector<double> getIntervalValues() const override {
        return {this->frobenius2, this->frobenius2min, this->frobenius2max};
    }
    [[nodiscard]] std::vector<std::string> getNominalValues() const override { return {}; }
    [[nodiscard]] std::string getName() const override { return "rotation matrix drift"; }
};


#endif //RAMPACK_ROTATIONMATRIXDRIFT_H
